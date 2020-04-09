#pragma once
#include "rendering/resource/DescPoolAllocator.h"
#include "rendering/objects/GBuffer.h"

#include <vulkan/vulkan.hpp>
namespace vl {
class DeferredPass {

public:
	// pipeline stuffs
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	void InitQuadDescriptor();
	void InitPipeline(vk::RenderPass renderPass);

	void RecordCmd(vk::CommandBuffer* cmdBuffer);
	void UpdateDescriptorSets(GBuffer& gbuffer);

protected:
	DescriptorLayout m_descLayout;

	vk::DescriptorSet m_descSet;


	vk::Viewport GetViewport() const;
	vk::Rect2D GetScissor() const;
};
} // namespace vl
