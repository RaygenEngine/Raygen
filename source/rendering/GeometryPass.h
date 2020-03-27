#pragma once
#include "rendering/asset/Model.h"
#include "rendering/PoolAllocator.h"
#include "rendering/wrapper/GBuffer.h"

#include <vulkan/vulkan.hpp>

namespace vl {

class GeometryPass {
	vk::UniqueRenderPass m_renderPass;
	vk::UniqueFramebuffer m_framebuffer;


	// pipeline stuffs
	// GENERIC MODEL GEOMETRY PASS PIPELINE
	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	// Model descriptors
	R_DescriptorLayout m_materialDescLayout;

public:
	void InitAll();

	void InitRenderPass();
	void InitFramebuffers();

	void MakePipeline();

	void RecordGeometryDraw(vk::CommandBuffer* cmdBuffer);

	// void TransitionGBufferForShaderRead();
	// void TransitionGBufferForAttachmentWrite();
	UniquePtr<GBuffer> m_gBuffer;

	vk::DescriptorSet GetMaterialDescriptorSet() const;

protected:
	vk::Viewport GetViewport() const;
	vk::Rect2D GetScissor() const;
};

} // namespace vl
