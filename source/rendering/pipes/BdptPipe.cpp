#include "BdptPipe.h"

#include "rendering/Renderer.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneCamera.h"

namespace {
struct PushConstant {
	int32 bounces;
	int32 frame;
	int32 pointlightCount;
	int32 spotlightCount;
	int32 dirlightCount;
	int32 quadlightCount;
};

static_assert(sizeof(PushConstant) <= 128);
} // namespace

namespace vl {
vk::UniquePipelineLayout BdptPipe::MakePipelineLayout()
{
	return rvk::makePipelineLayoutEx(
		{
			DescriptorLayouts->_1storageImage.handle(),                   // images
			DescriptorLayouts->_1uniformBuffer.handle(),                  // camera
			DescriptorLayouts->accelerationStructure.handle(),            // as
			DescriptorLayouts->_1storageBuffer_1024samplerImage.handle(), // geometry and texture
			DescriptorLayouts->_1storageBuffer.handle(),                  // pointlights
			DescriptorLayouts->_1storageBuffer_1024samplerImage.handle(), // spotlights
			DescriptorLayouts->_1storageBuffer_1024samplerImage.handle(), // dirlights
			DescriptorLayouts->_1storageBuffer.handle()                   // quadlights
		},
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, sizeof(PushConstant));
}

vk::UniquePipeline BdptPipe::MakePipeline()
{
	auto getShader = [](const auto& path) -> auto&
	{
		GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader(path);
		gpuShader.onCompile = [&]() {
			StaticPipes::Recompile<BdptPipe>();
		};
		return gpuShader;
	};

	// all rt shaders here
	auto& ptshader = getShader("engine-data/spv/pathtrace/bidirectional/bdpt.shader");
	auto& ptLightPathShader = getShader("engine-data/spv/pathtrace/bidirectional/bdpt-lightpath.shader");
	auto& ptQuadlightShader = getShader("engine-data/spv/pathtrace/bidirectional/bdpt-quadlight.shader");
	auto& ptMergePathShader = getShader("engine-data/spv/pathtrace/bidirectional/bdpt-merge.shader");
	auto& ptMergePathQuadlightShader = getShader("engine-data/spv/pathtrace/bidirectional/bdpt-merge-quadlight.shader");

	auto get = [](auto shader) {
		return *shader.Lock().module;
	};

	AddRaygenGroup(get(ptshader.rayGen));
	AddMissGroup(get(ptshader.miss));                                              // miss general 0
	AddMissGroup(get(ptLightPathShader.miss));                                     // miss lightpath 1
	AddMissGroup(get(ptMergePathShader.miss));                                     // miss merge 2
	AddHitGroup(get(ptshader.closestHit), get(ptshader.anyHit));                   // gltf mat 0, ahit for mask
	AddHitGroup(get(ptQuadlightShader.closestHit));                                // quad lights 0 + 1
	AddHitGroup(get(ptLightPathShader.closestHit));                                // lightpath 2
	AddHitGroup(get(ptMergePathShader.closestHit), get(ptMergePathShader.anyHit)); // merge 3, ahit for mask
	AddHitGroup(get(ptMergePathQuadlightShader.closestHit));                       // merge quadlight 3 + 1

	vk::RayTracingPipelineCreateInfoKHR rayPipelineInfo{};
	rayPipelineInfo
		.setLayout(layout()) //
		.setMaxPipelineRayRecursionDepth(1);

	// Assemble the shader stages and construct the SBT
	return MakeRtPipeline(rayPipelineInfo);
}

void BdptPipe::RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, const vk::Extent3D& extent,
	vk::DescriptorSet storageImagesDescSet, int32 frame, int32 bounces) const
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	cmdBuffer.bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, pipeline());

	cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, layout(), 0u,
		{
			storageImagesDescSet,
			sceneDesc.viewer.uboDescSet[sceneDesc.frameIndex],
			sceneDesc.scene->sceneAsDescSet,
			sceneDesc.scene->tlas.sceneDesc.descSetGeometryAndTextures[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetPointlights[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetSpotlights[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetDirlights[sceneDesc.frameIndex],
			sceneDesc.scene->tlas.sceneDesc.descSetQuadlights[sceneDesc.frameIndex],
		},
		nullptr);

	PushConstant pc{
		bounces,
		frame,
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
