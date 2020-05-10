#pragma once
#include "rendering/objects/ImageAttachment.h"
#include "rendering/Device.h"

#include <vulkan/vulkan.hpp>

namespace vl {
struct RDepthmap {

	vk::UniqueFramebuffer framebuffer;
	UniquePtr<ImageAttachment> attachment;

	vk::DescriptorSet descSet;

public:
	RDepthmap(vk::RenderPass renderPass, uint32 width, uint32 height, const char* name = "depthmap");

	// DOC: transitioning for read is generally done automatically from the render pass
	// if need be create a transition for read method
	void TransitionForWrite(vk::CommandBuffer* cmdBuffer);
};
} // namespace vl
