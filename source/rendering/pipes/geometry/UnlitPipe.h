#pragma once
#include "rendering/pipes/StaticPipeBase.h"

namespace vl {
class UnlitPipe {

public:
	static size_t GetPushConstantSize();

	static vk::UniquePipeline CreatePipeline(vk::PipelineLayout pipelineLayout, //
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	// static vk::UniquePipeline CreateAnimPipeline(
	//	vk::PipelineLayout pipelineLayout, std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

	static void RecordCmd(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc);
};


} // namespace vl
