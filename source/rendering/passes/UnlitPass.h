#pragma once
#include "rendering/wrappers/RGbuffer.h"
#include "rendering/scene/SceneGeometry.h"

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

	static void RecordCmd(vk::CommandBuffer* cmdBuffer, vk::Extent2D extent, //
		const std::vector<SceneGeometry*>& geometries, const std::vector<SceneAnimatedGeometry*>& animGeometries);
};

} // namespace vl