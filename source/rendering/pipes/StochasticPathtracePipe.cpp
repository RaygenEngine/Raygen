#include "StochasticPathtracePipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneCamera.h"


namespace {
struct PushConstant {
	int32 seed;
	int32 samples;
	int32 bounces;
	int32 pointlightCount;
	int32 spotlightCount;
	int32 dirlightCount;
	int32 quadlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout StochasticPathtracePipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->_1storageImage.handle(),                   // images
			DescriptorLayouts->_1uniformBuffer.handle(),                  // viewer
			DescriptorLayouts->accelerationStructure.handle(),            // as
			DescriptorLayouts->_1storageBuffer_1024samplerImage.handle(), // geometry and texture
			DescriptorLayouts->_1storageBuffer.handle(),                  // pointlights
			DescriptorLayouts->_1storageBuffer_1024samplerImage.handle(), // spotlights
			DescriptorLayouts->_1storageBuffer_1024samplerImage.handle(), // dirlights
			DescriptorLayouts->_1storageBuffer.handle()                   // quadlights
		},
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR);
}

vk::UniquePipeline StochasticPathtracePipe::MakePipeline()
{
	auto getShader = [](const auto& path) -> auto&
	{
		GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader(path);
		gpuShader.onCompile = [&]() {
			StaticPipes::Recompile<StochasticPathtracePipe>();
		};
		return gpuShader;
	};

	// all rt shaders here
	auto& ptshader = getShader("engine-data/spv/pathtrace/stochastic/spt.shader");
	auto& ptQuadlightShader = getShader("engine-data/spv/pathtrace/stochastic/spt-quadlight.shader");
	auto& ptShadowShadow = getShader("engine-data/spv/pathtrace/stochastic/spt-shadow.shader");
	auto& ptShadowQuadlightShadow = getShader("engine-data/spv/pathtrace/stochastic/spt-shadow-quadlight.shader");

	auto get = [](auto shader) {
		return *shader.Lock().module;
	};

	AddRaygenGroup(get(ptshader.rayGen));
	AddMissGroup(get(ptshader.miss));                                        // miss general 0
	AddMissGroup(get(ptShadowShadow.miss));                                  // miss shadow 1
	AddHitGroup(get(ptshader.closestHit), get(ptshader.anyHit));             // gltf mat 0, ahit for mask
	AddHitGroup(get(ptQuadlightShader.closestHit));                          // quad lights 1
	AddHitGroup(get(ptShadowShadow.closestHit), get(ptShadowShadow.anyHit)); // shadow 2, ahit for mask
	AddHitGroup(get(ptShadowQuadlightShadow.closestHit));                    // shadow quad lights 3

	vk::RayTracingPipelineCreateInfoKHR rayPipelineInfo{};
	rayPipelineInfo
		.setLayout(layout()) //
		.setMaxPipelineRayRecursionDepth(2);

	// Assemble the shader stages and construct the SBT
	return MakeRaytracingPipeline(rayPipelineInfo);
}

void StochasticPathtracePipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
	const vk::Extent3D& extent, vk::DescriptorSet storageImageDescSet, vk::DescriptorSet viewerDescSet, int32 seed,
	int32 samples, int32 bounces) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 0u,
		{
			storageImageDescSet,
			viewerDescSet,
			sceneDesc.scene->sceneAsDescSet,
			sceneDesc.scene->tlas.sceneDesc.descSetGeometryAndTextures[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetPointlights[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetSpotlights[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetDirlights[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex],
		},
		nullptr);

	PushConstant pc{
		seed,
		samples,
		bounces,
		sceneDesc.scene->tlas.sceneDesc.pointlightCount,
		sceneDesc.scene->tlas.sceneDesc.spotlightCount,
		sceneDesc.scene->tlas.sceneDesc.dirlightCount,
		sceneDesc.scene->tlas.sceneDesc.quadlightCount,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u,
		sizeof(PushConstant), &pc);

	TraceRays(cmdBuffer, extent.width, extent.height, 1u);
}
} // namespace vl
