#pragma once
#include "rendering/resource/DescPoolAllocator.h"
#include "rendering/objects/GBuffer.h"

#include <vulkan/vulkan.hpp>
namespace vl {
class SpotlightPass {

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

public:
	void MakePipeline(vk::RenderPass renderPass);

	void RecordCmd(vk::CommandBuffer* cmdBuffer, const vk::Viewport& viewport, const vk::Rect2D& scissor);
};
} // namespace vl
