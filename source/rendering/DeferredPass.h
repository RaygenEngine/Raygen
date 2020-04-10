#pragma once
#include "rendering/resource/DescPoolAllocator.h"
#include "rendering/objects/GBuffer.h"

#include <vulkan/vulkan.hpp>
namespace vl {
class DeferredPass {

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	DescriptorLayout m_gBufferDescLayout;
	vk::DescriptorSet m_gBufferDescSet;

public:
	DeferredPass(vk::RenderPass renderPass);

	void MakePipeline(vk::RenderPass renderPass);

	void RecordCmd(vk::CommandBuffer* cmdBuffer);

	void UpdateDescriptorSet(GBuffer& gbuffer);

protected:
	vk::Viewport GetViewport() const;
	vk::Rect2D GetScissor() const;
};
} // namespace vl
