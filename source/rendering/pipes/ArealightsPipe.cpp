#include "ArealightsPipe.h"

#include "engine/console/ConsoleVariable.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/core/PipeUtl.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/Scene.h"
#include "rendering/util/WriteDescriptorSets.h"

namespace {
struct PushConstant {
	int32 frame;
	int32 quadlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout ArealightsPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayoutEx(
		{
			Layouts->globalDescLayout.handle(),            // gbuffer and stuff
			Layouts->doubleStorageImage.handle(),          // image result
			Layouts->accelLayout.handle(),                 // accel structure
			Layouts->bufferAndSamplersDescLayout.handle(), // geometry and texture
			Layouts->singleStorageBuffer.handle(),         // quadlights
		},
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, sizeof(PushConstant));
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
	auto& ptshader = getShader("engine-data/spv/raytrace/arealights/arlights.shader");
	auto& ptQuadlightShader = getShader("engine-data/spv/raytrace/arealights/arlights-quadlight.shader");

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

void ArealightsPipe::Draw(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc,
	vk::DescriptorSet storageImagesDescSet, const vk::Extent3D& extent, int32 frame) const
{
	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 0u, 1u, &sceneDesc.globalDesc, 0u,
		nullptr); // gbuffer and stuff

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 1u, 1u, &storageImagesDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, layout(), 2u, 1u, &sceneDesc.scene->sceneAsDescSet, 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 3u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetGeometryAndTextures[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 4u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex], 0u, nullptr);


	PushConstant pc{
		frame,
		sceneDesc.scene->tlas.sceneDesc.quadlightCount,
	};

	cmdBuffer.pushConstants(layout(), vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u,
		sizeof(PushConstant), &pc);

	cmdBuffer.traceRaysKHR(&m_raygenShaderBindingTable, &m_missShaderBindingTable, &m_hitShaderBindingTable,
		&m_callableShaderBindingTable, extent.width, extent.height, 1);
}

} // namespace vl
