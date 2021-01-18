#include "PathtraceCubemapPipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/core/PipeUtl.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneReflProbe.h"

namespace {
struct PushConstant {
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
vk::UniquePipelineLayout PathtraceCubemapPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayoutEx(
		{
			Layouts->doubleStorageImage.handle(),          // images
			Layouts->singleUboDescLayout.handle(),         // camera
			Layouts->accelLayout.handle(),                 // as
			Layouts->bufferAndSamplersDescLayout.handle(), // geometry and texture
			Layouts->singleStorageBuffer.handle(),         // pointlights
			Layouts->bufferAndSamplersDescLayout.handle(), // spotlights
			Layouts->bufferAndSamplersDescLayout.handle(), // dirlights
			Layouts->singleStorageBuffer.handle()          // quadlights
		},
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, sizeof(PushConstant));
}

vk::UniquePipeline PathtraceCubemapPipe::MakePipeline()
{
	auto getShader = [](const auto& path) -> auto&
	{
		GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader(path);
		gpuShader.onCompile = [&]() {
			StaticPipes::Recompile<PathtraceCubemapPipe>();
		};
		return gpuShader;
	};

	// all rt shaders here
	auto& ptshader = getShader("engine-data/spv/pathtrace/cubemap/cube.shader");
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
		.setMaxPipelineRayRecursionDepth(1);

	// Assemble the shader stages and construct the SBT
	return MakeRtPipeline(rayPipelineInfo);
}

void PathtraceCubemapPipe::Draw(
	vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const SceneReflprobe& rp) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 0u, 1u, &rp.environmentStorageDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 1u, 1u, &rp.uboDescSet[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 2u, 1u, &sceneDesc.scene->sceneAsDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 3u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetGeometryAndTextures[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 4u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetPointlights[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 5u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetSpotlights[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 6u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetDirlights[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 7u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex], 0u, nullptr);

	PushConstant pc{
		rp.ptSamples,
		rp.ptBounces,
		sceneDesc.scene->tlas.sceneDesc.pointlightCount,
		sceneDesc.scene->tlas.sceneDesc.spotlightCount,
		sceneDesc.scene->tlas.sceneDesc.dirlightCount,
		sceneDesc.scene->tlas.sceneDesc.quadlightCount,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u,
		sizeof(PushConstant), &pc);

	cmdBuffer.traceRaysKHR(&m_raygenShaderBindingTable, &m_missShaderBindingTable, &m_hitShaderBindingTable,
		&m_callableShaderBindingTable, rp.environment.extent.width, rp.environment.extent.height, 1);
}
} // namespace vl
