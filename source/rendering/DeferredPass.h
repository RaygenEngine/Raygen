#pragma once
#include "rendering/objects/GBuffer.h"
#include "rendering/resource/DescPoolAllocator.h"
#include "rendering/SpotlightsPass.h"

#include <vulkan/vulkan.hpp>
namespace vl {
class DeferredPass {

	DescriptorLayout m_gBufferDescLayout;
	vk::DescriptorSet m_gBufferDescSet;

	SpotlightsPass m_slPass;

public:
	DeferredPass(vk::RenderPass renderPass);

	void RecordCmd(vk::CommandBuffer* cmdBuffer);

	void UpdateGBufferDescriptorSet(GBuffer& gbuffer);

protected:
	vk::Viewport GetViewport() const;
	vk::Rect2D GetScissor() const;
};
} // namespace vl
