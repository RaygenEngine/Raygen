#pragma once
#include "rendering/objects/Image.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class Cubemap : public Image {

public:
	Cubemap(uint32 dims, vk::Format format, vk::ImageTiling tiling, vk::ImageLayout initalLayout,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);


	void Cubemap::CopyBufferToFace(const RawBuffer& buffer, uint32 face);
};
} // namespace vl
