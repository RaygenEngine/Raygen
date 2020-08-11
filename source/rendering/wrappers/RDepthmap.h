#pragma once
#include "rendering/wrappers/RImageAttachment.h"

namespace vl {
struct RDepthmap {

	vk::UniqueFramebuffer framebuffer;
	UniquePtr<RImageAttachment> attachment;

	// sampler2DShadow (this may require separate class later)
	vk::UniqueSampler depthSampler;

	vk::DescriptorSet descSet;

	RDepthmap(uint32 width, uint32 height, const char* name = "depthmap");
};
} // namespace vl
