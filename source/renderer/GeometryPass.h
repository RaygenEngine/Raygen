#pragma once
#include "renderer/Model.h"

#include <vulkan/vulkan.hpp>

class GeometryPass {
public:
	vk::UniqueRenderPass m_renderPass;
	vk::UniqueFramebuffer m_framebuffer;

	std::unique_ptr<Image> albedoImage;
	vk::UniqueImageView albedoImageView;

	std::unique_ptr<Image> depthImage;
	vk::UniqueImageView depthImageView;

	// pipeline stuffs
	// GENERIC MODEL GEOMETRY PASS PIPELINE
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	void InitRenderPass();
	void InitFramebuffers();
	void InitPipelineAndStuff();

	void RecordGeometryDraw(vk::CommandBuffer* cmdBuffer);

	void TransitionGBufferForShaderRead();
	void TransitionGBufferForAttachmentWrite();

protected:
	vk::Viewport GetViewport() const;
	vk::Rect2D GetScissor() const;
};
