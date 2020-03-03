#pragma once


#include "renderer/Model.h"
#include <vulkan/vulkan.hpp>

class GeometryPass {
public:
	vk::UniqueRenderPass m_renderPass;
	vk::UniqueFramebuffer m_framebuffer;

	vk::UniqueImage albedoImage;
	vk::UniqueDeviceMemory albedoImageMemory;
	vk::UniqueImageView albedoImageView;

	vk::UniqueImage depthImage;
	vk::UniqueDeviceMemory depthImageMemory;
	vk::UniqueImageView depthImageView;

	// pipeline stuffs
	// GENERIC MODEL GEOMETRY PASS PIPELINE
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;


	void CreateFramebufferImageViews();


	void InitRenderPassAndFramebuffers();
	void InitPipelineAndStuff();

	void RecordGeometryDraw(vk::CommandBuffer* cmdBuffer);

	void TransitionGBufferForShaderRead();
	void TransitionGBufferForAttachmentWrite();
};
