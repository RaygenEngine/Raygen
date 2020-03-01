#pragma once


#include "renderer/Model.h"
#include <vulkan/vulkan.hpp>

// WIP:
class DeferredPass {
public:
	vk::UniqueRenderPass m_renderPass;
	std::vector<vk::UniqueFramebuffer> m_framebuffers;


	// pipeline stuffs
	// GENERIC MODEL GEOMETRY PASS PIPELINE
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;


	void InitRenderPassAndFramebuffers();
	void InitPipelineAndStuff();

	void RecordCmd(vk::CommandBuffer* cmdBuffer, vk::Framebuffer& framebuffer);
};
