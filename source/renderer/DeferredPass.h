#pragma once


#include "renderer/Model.h"
#include <vulkan/vulkan.hpp>

// WIP:
class DeferredPass {
public:
	// pipeline stuffs
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	void InitPipeline(vk::RenderPass& renderPass);

	void RecordCmd(vk::CommandBuffer* cmdBuffer);
};
