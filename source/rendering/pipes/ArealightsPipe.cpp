#include "ArealightsPipe.h"

#include "rendering/Layouts.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"

namespace {
struct PushConstant {
	int32 frame;
	int32 samples;
	int32 quadlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace


namespace vl {
vk::UniquePipelineLayout ArealightsPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayout<PushConstant>(
		{
			DescriptorLayouts->global.handle(),                           // gbuffer and stuff
			DescriptorLayouts->_1storageImage.handle(),                   // image result
			DescriptorLayouts->accelerationStructure.handle(),            // accel structure
			DescriptorLayouts->_1storageBuffer_1024samplerImage.handle(), // geometry and texture
			DescriptorLayouts->_1storageBuffer.handle(),                  // quadlights
		},
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR);
}

vk::UniquePipeline ArealightsPipe::MakePipeline()
{
	auto getShader = [](const auto& path) -> auto&
	{
		GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader(path);
		gpuShader.onCompile = [&]() {
			StaticPipes::Recompile<ArealightsPipe>();
		};
		return gpuShader;
	};

	// all rt shaders here
	auto& ptshader = getShader("engine-data/spv/raytrace/arealights/arealights.shader");
	auto& ptQuadlightShader = getShader("engine-data/spv/raytrace/arealights/arealights-quadlight.shader");

	auto get = [](auto shader) {
		return *shader.Lock().module;
	};

	AddRaygenGroup(get(ptshader.rayGen));
	AddMissGroup(get(ptshader.miss));                            // miss general 0
	AddHitGroup(get(ptshader.closestHit), get(ptshader.anyHit)); // gltf mat 0, ahit for mask
	AddHitGroup(get(ptQuadlightShader.closestHit));              // quad lights 1

	vk::RayTracingPipelineCreateInfoKHR rayPipelineInfo{};
	rayPipelineInfo
		.setLayout(layout()) //
		.setMaxPipelineRayRecursionDepth(1);

	// Assemble the shader stages and construct the SBT
	return MakeRaytracingPipeline(rayPipelineInfo);
}

void ArealightsPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
	const vk::Extent3D& extent, vk::DescriptorSet storageImagesDescSet, int32 samples, int32 seed) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	if (!sceneDesc.scene->tlas.sceneDesc.quadlightCount) {
		return;
	}

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 0u,
		{
			sceneDesc.globalDesc,
			storageImagesDescSet,
			sceneDesc.scene->sceneAsDescSet,
			sceneDesc.scene->tlas.sceneDesc.descSetGeometryAndTextures[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex],
		},
		nullptr);

	PushConstant pc{
		.frame = seed,
		.samples = samples,
		.quadlightCount = sceneDesc.scene->tlas.sceneDesc.quadlightCount,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u,
		sizeof(PushConstant), &pc);

	TraceRays(cmdBuffer, extent.width, extent.height, 1u);
}

} // namespace vl
