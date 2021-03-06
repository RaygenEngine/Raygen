#pragma once
#include "rendering/scene/Scene.h"

namespace vl {
class DepthmapPipe {

public:
	static size_t GetPushConstantSize();

	static vk::UniquePipeline CreatePipeline(vk::PipelineLayout pipelineLayout, //
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	static vk::UniquePipeline CreateAnimPipeline(
		vk::PipelineLayout pipelineLayout, std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	static void RecordCmd(vk::CommandBuffer cmdBuffer, const glm::mat4& viewProj, const SceneRenderDesc& sceneDesc);
};

} // namespace vl
