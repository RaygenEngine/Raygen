#include "TestSptPipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/Scene.h"


namespace {
struct PushConstant {
	int32 frame;
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
vk::UniquePipelineLayout TestSptPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayoutEx(
		{
			Layouts->globalDescLayout.handle(),            // gbuffer and stuff
			Layouts->tripleStorageImage.handle(),          // images
			Layouts->accelLayout.handle(),                 // as
			Layouts->bufferAndSamplersDescLayout.handle(), // geometry and texture TODO: enforce meta (globals = set0,
														   // geom/textures = set3) with functions
			Layouts->singleStorageBuffer.handle(),         // pointlights
			Layouts->bufferAndSamplersDescLayout.handle(), // spotlights
			Layouts->bufferAndSamplersDescLayout.handle(), // dirlights
			Layouts->singleStorageBuffer.handle()          // quadlights
		},
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, sizeof(PushConstant));
}

vk::UniquePipeline TestSptPipe::MakePipeline()
{
	auto getShader = [](const auto& path) -> auto&
	{
		GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader(path);
		gpuShader.onCompile = [&]() {
			StaticPipes::Recompile<TestSptPipe>();
		};
		return gpuShader;
	};

	// all rt shaders here
	auto& ptshader = getShader("engine-data/spv/raytrace/spt/spt.shader");
	auto& ptQuadlightShader = getShader("engine-data/spv/raytrace/spt/spt-quadlight.shader");
	auto& ptShadowShadow = getShader("engine-data/spv/raytrace/spt/spt-shadow.shader");
	auto& ptShadowQuadlightShadow = getShader("engine-data/spv/raytrace/spt/spt-shadow-quadlight.shader");

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
	return MakeRtPipeline(rayPipelineInfo);
}

void TestSptPipe::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
	vk::DescriptorSet storageImagesDescSet, const vk::Extent3D& extent, int32 iteration, int32 samples,
	int32 bounces) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 0u,
		{
			sceneDesc.globalDesc,
			storageImagesDescSet,
			sceneDesc.scene->sceneAsDescSet,
			sceneDesc.scene->tlas.sceneDesc.descSetGeometryAndTextures[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetPointlights[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetSpotlights[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetDirlights[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex],
		},
		nullptr);

	PushConstant pc{
		iteration,
		samples,
		bounces,
		sceneDesc.scene->tlas.sceneDesc.pointlightCount,
		sceneDesc.scene->tlas.sceneDesc.spotlightCount,
		sceneDesc.scene->tlas.sceneDesc.dirlightCount,
		sceneDesc.scene->tlas.sceneDesc.quadlightCount,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u,
		sizeof(PushConstant), &pc);

	cmdBuffer.traceRaysKHR(&m_raygenShaderBindingTable, &m_missShaderBindingTable, &m_hitShaderBindingTable,
		&m_callableShaderBindingTable, extent.width, extent.height, 1);
}
} // namespace vl
