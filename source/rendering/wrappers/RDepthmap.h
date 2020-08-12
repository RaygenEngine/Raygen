#pragma once
#include "rendering/wrappers/RImage.h"

namespace vl {
struct RDepthmap {

	vk::UniqueFramebuffer framebuffer;
	RImageAttachment attachment;

	// sampler2DShadow (this may require separate class later)
	vk::UniqueSampler depthSampler;

	vk::DescriptorSet descSet;

	RDepthmap() = default;
	RDepthmap(uint32 width, uint32 height, const char* name = "depthmap");
};
} // namespace vl
