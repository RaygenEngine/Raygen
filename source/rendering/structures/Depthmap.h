#pragma once
#include "rendering/wrappers/Framebuffer.h"

namespace vl {
struct Depthmap {

	RFramebuffer framebuffer;

	// sampler2DShadow (this may require separate class later)
	vk::UniqueSampler depthSampler;
	vk::DescriptorSet descSet;

	Depthmap() = default;
	Depthmap(uint32 width, uint32 height, const char* name = "depthmap");
};
} // namespace vl
