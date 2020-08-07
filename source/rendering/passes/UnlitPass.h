#pragma once
#include "rendering/wrappers/RGbuffer.h"
#include "rendering/scene/Scene.h"

#include <vulkan/vulkan.hpp>

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
	static void RecordCmd(vk::CommandBuffer* cmdBuffer, vk::Extent2D extent, SceneRenderDesc& sceneDesc);
};

} // namespace vl
