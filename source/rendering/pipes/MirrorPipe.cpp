#include "MirrorPipe.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/core/PipeUtl.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"

ConsoleVariable<int32> cons_mirrorDepth{ "r.mirror.depth", 1, "Set mirror depth" };

namespace {
struct PushConstant {
	int32 depth;
	int32 pointlightCount;
	int32 spotlightCount;
	int32 dirlightCount;
	int32 irragridCount;
	int32 quadlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout MirrorPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayoutEx(
		{
			Layouts->globalDescLayout.handle(),            // gbuffer and stuff
			Layouts->singleStorageImage.handle(),          // images
			Layouts->accelLayout.handle(),                 // as
			Layouts->bufferAndSamplersDescLayout.handle(), // geometry and texture
			Layouts->singleStorageBuffer.handle(),         // pointlights
			Layouts->bufferAndSamplersDescLayout.handle(), // spotlights
			Layouts->bufferAndSamplersDescLayout.handle(), // dirlights
			Layouts->bufferAndSamplersDescLayout.handle(), // irragrids
			Layouts->singleStorageBuffer.handle()          // quadlights
		},
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, sizeof(PushConstant));
}

vk::UniquePipeline MirrorPipe::MakePipeline()
{
	auto getShader = [](const auto& path) -> auto&
	{
		GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader(path);
		gpuShader.onCompile = [&]() {
			StaticPipes::Recompile<MirrorPipe>();
		};
		return gpuShader;
	};

	// all rt shaders here
	auto& ptshader = getShader("engine-data/spv/raytrace/mirror/mirror.shader");
	auto& ptQuadlightShader = getShader("engine-data/spv/raytrace/mirror/mirror-quadlight.shader");

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
	return MakeRtPipeline(rayPipelineInfo);
}

void MirrorPipe::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
	vk::DescriptorSet mirrorImageStorageDescSet, const vk::Extent3D& extent) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline());

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 0u, 1u, &sceneDesc.globalDesc, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 1u, 1u, &mirrorImageStorageDescSet, 0u, nullptr);

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
		&sceneDesc.scene->tlas.sceneDesc.descSetIrragrids[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 8u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex], 0u, nullptr);

	PushConstant pc{
		std::max(0, *cons_mirrorDepth),
		sceneDesc.scene->tlas.sceneDesc.pointlightCount,
		sceneDesc.scene->tlas.sceneDesc.spotlightCount,
		sceneDesc.scene->tlas.sceneDesc.dirlightCount,
		sceneDesc.scene->tlas.sceneDesc.irragridCount,
		sceneDesc.scene->tlas.sceneDesc.quadlightCount,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u,
		sizeof(PushConstant), &pc);

	cmdBuffer.traceRaysKHR(&m_raygenShaderBindingTable, &m_missShaderBindingTable, &m_hitShaderBindingTable,
		&m_callableShaderBindingTable, extent.width, extent.height, 1);
}

} // namespace vl
