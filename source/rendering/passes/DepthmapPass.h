#pragma once
#include "rendering/scene/Scene.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class DepthmapPass {

public:
	static vk::UniqueRenderPass CreateCompatibleRenderPass();

	static size_t GetPushConstantSize();

	static vk::UniquePipeline CreatePipeline(vk::PipelineLayout pipelineLayout, //
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	static vk::UniquePipeline CreateAnimPipeline(
		vk::PipelineLayout pipelineLayout, std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	static void RecordCmd(vk::CommandBuffer* cmdBuffer, vk::Viewport viewport, vk::Rect2D scissor,
		const glm::mat4& viewProj, const SceneRenderDesc& sceneDesc);
};

} // namespace vl
