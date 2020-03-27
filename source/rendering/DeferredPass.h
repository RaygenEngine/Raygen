#pragma once
#include <vulkan/vulkan.hpp>
namespace vl {

class DeferredPass {
public:
	// pipeline stuffs
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	void InitPipeline(vk::RenderPass renderPass);

	void RecordCmd(vk::CommandBuffer* cmdBuffer);


protected:
	vk::Viewport GetViewport() const;
	vk::Rect2D GetScissor() const;
};
} // namespace vl
