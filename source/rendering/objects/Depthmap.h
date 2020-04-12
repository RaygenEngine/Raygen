#pragma once
#include "rendering/objects/ImageAttachment.h"
#include "rendering/Device.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class Depthmap {

	vk::UniqueFramebuffer m_framebuffer;
	UniquePtr<ImageAttachment> m_depthAttachment;

public:
	Depthmap(vk::RenderPass renderPass, uint32 width, uint32 height, const char* name = "depthmap");

	// DOC: transitioning for read is generally done automatically from the render pass
	// if need be create a transition for read method
	void TransitionForWrite(vk::CommandBuffer* cmdBuffer);

	[[nodiscard]] vk::Framebuffer GetFramebuffer() const { return m_framebuffer.get(); }
	[[nodiscard]] ImageAttachment* GetDepthAttachment() const { return m_depthAttachment.get(); }
};
} // namespace vl
