#pragma once
#include "rendering/wrappers/Framebuffer.h"

namespace vl {
struct Depthmap {

	RFramebuffer framebuffer;

	vk::Sampler depthSampler;
	vk::DescriptorSet descSet;

	Depthmap() = default;
	Depthmap(uint32 width, uint32 height, const char* name = "depthmap");

	Depthmap(Depthmap const&) = delete;
	// CHECK:
	Depthmap(Depthmap&&) = delete;
	Depthmap& operator=(Depthmap const&) = delete;
	Depthmap& operator=(Depthmap&& other);


	~Depthmap();
};
} // namespace vl
