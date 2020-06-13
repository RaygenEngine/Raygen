#pragma once
#include "rendering/objects/RImage.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class RImage2D : public RImage {

public:
	RImage2D(uint32 width, uint32 height, uint32 mipLevels, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
};
} // namespace vl
