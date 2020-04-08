#pragma once
#include "rendering/objects/Image.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class Image2D : public Image {

public:
	Image2D(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageLayout initalLayout,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);
};
} // namespace vl
