#pragma once
#include "rendering/resource/DescPoolAllocator.h"
#include "rendering/objects/GBuffer.h"

#include <vulkan/vulkan.hpp>
namespace vl {
class AmbientPass {

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

public:
	void MakePipeline();

	void RecordCmd(vk::CommandBuffer* cmdBuffer, const vk::Viewport& viewport, const vk::Rect2D& scissor);
};
} // namespace vl
