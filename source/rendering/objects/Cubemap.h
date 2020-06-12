#pragma once
#include "rendering/objects/Image.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class Cubemap : public Image {

public:
	Cubemap(uint32 dims, uint32 mipCount, vk::Format format, vk::ImageTiling tiling, vk::ImageLayout initalLayout,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);


	void Cubemap::CopyBufferToFace(const RBuffer& buffer, uint32 face, uint32 mip);

	void Cubemap::CopyBuffer(const RBuffer& buffer, size_t pixelSize, uint32 mipCount);
};
} // namespace vl
