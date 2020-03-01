#pragma once


#include "renderer/Model.h"
#include <vulkan/vulkan.hpp>

// WIP:
class EditorPass {
public:
	vk::UniqueRenderPass m_renderPass;
	std::vector<vk::UniqueFramebuffer> m_framebuffers;


	// pipeline stuffs
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;


	void InitRenderPassAndFramebuffers();

	void RecordCmd(vk::CommandBuffer* cmdBuffer, vk::Framebuffer& framebuffer);
};
