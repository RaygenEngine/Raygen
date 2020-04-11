#pragma once
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/objects/GBuffer.h"
#include "rendering/resource/GpuResources.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class ShadowmapPass {
	vk::UniqueRenderPass m_renderPass;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;

public:
	ShadowmapPass();
	void MakePipeline();

	void RecordCmd(vk::CommandBuffer* cmdBuffer, const vk::Viewport& viewport, const vk::Rect2D& scissor);

	[[nodiscard]] vk::RenderPass GetRenderPass() const { return m_renderPass.get(); }
};

} // namespace vl
