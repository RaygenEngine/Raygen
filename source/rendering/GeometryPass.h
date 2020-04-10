#pragma once
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/objects/GBuffer.h"

#include <vulkan/vulkan.hpp>

namespace vl {

class GeometryPass {
	vk::UniqueRenderPass m_renderPass;
	vk::UniqueFramebuffer m_framebuffer;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	DescriptorLayout m_regularMaterialDescLayout;

	void InitRenderPass();

public:
	GeometryPass();

	void MakePipeline();
	void MakeFramebuffers(GBuffer& gbuffer);

	void RecordGeometryDraw(vk::CommandBuffer* cmdBuffer);

	vk::DescriptorSet GetRegularMaterialDescriptorSet() const;

protected:
	vk::Viewport GetViewport() const;
	vk::Rect2D GetScissor() const;
};

} // namespace vl
