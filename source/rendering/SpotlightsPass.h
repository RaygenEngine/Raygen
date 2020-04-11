#pragma once
#include "rendering/resource/DescPoolAllocator.h"
#include "rendering/objects/GBuffer.h"

#include <vulkan/vulkan.hpp>
namespace vl {
class SpotlightsPass {

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	DescriptorLayout& m_gBufferDescLayout;
	vk::DescriptorSet m_gBufferDescSet;

public:
	SpotlightsPass(vk::RenderPass renderPass, std::pair<DescriptorLayout&, vk::DescriptorSet&> gBufferDescs);

	void MakePipeline(vk::RenderPass renderPass);

	void RecordCmd(vk::CommandBuffer* cmdBuffer, const vk::Viewport& viewport, const vk::Rect2D& scissor);
};
} // namespace vl
