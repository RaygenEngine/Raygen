#pragma once


#include "renderer/Model.h"
#include <vulkan/vulkan.hpp>

struct GBufferAttachment {
	vk::UniqueImage image;
	vk::UniqueDeviceMemory memory;
	vk::UniqueImageView view;
};

struct GBuffer {
	GBufferAttachment position;
	GBufferAttachment normal;
	// rgb: albedo, a: opacity
	GBufferAttachment albedo;
	// r: metallic, g: roughness, b: occlusion, a: occlusion strength
	GBufferAttachment specular;
	GBufferAttachment emissive;
	GBufferAttachment depth;
};

// WIP:
class GeometryPass {
public:
	vk::UniqueRenderPass m_renderPass;
	vk::UniqueFramebuffer m_framebuffer;

	GBuffer gBuffer;

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
