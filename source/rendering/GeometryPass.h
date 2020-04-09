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

	UniquePtr<GBuffer> m_gBuffer;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

	DescriptorLayout m_materialDescLayout;

public:
	void InitAll();

	void InitRenderPass();
	void InitFramebuffers();

	void MakePipeline();

	void RecordGeometryDraw(vk::CommandBuffer* cmdBuffer);

	vk::DescriptorSet GetMaterialDescriptorSet() const;
	GBuffer* GetGBuffer() const { return m_gBuffer.get(); }

protected:
	vk::Viewport GetViewport() const;
	vk::Rect2D GetScissor() const;
};

} // namespace vl
