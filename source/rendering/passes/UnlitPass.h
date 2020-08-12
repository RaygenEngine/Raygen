#pragma once
#include "rendering/structures/GBuffer.h"
#include "rendering/scene/Scene.h"

namespace vl {
class UnlitPass {

public:
	// static vk::UniqueRenderPass CreateCompatibleRenderPass();

	static size_t GetPushConstantSize();

	static vk::UniquePipeline CreatePipeline(vk::PipelineLayout pipelineLayout, //
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	// static vk::UniquePipeline CreateAnimPipeline(
	//	vk::PipelineLayout pipelineLayout, std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	// WIP: extent
	static void RecordCmd(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc);
};

} // namespace vl
