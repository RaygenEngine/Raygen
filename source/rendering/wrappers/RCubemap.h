#pragma once
#include "rendering/wrappers/RImage.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class RCubemap : public RImage {

public:
	RCubemap(uint32 dims, uint32 mipCount, vk::Format format, vk::ImageTiling tiling, vk::ImageLayout initalLayout,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);

	void RCubemap::CopyBuffer(const RBuffer& buffer, size_t pixelSize, uint32 mipCount);
};
} // namespace vl
