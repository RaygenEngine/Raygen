#pragma once
#include "rendering/objects/Image.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class ImageAttachment : public Image {

	std::string m_name;

public:
	ImageAttachment(const std::string& name, uint32 width, uint32 height, vk::Format format,
		vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);

	[[nodiscard]] std::string GetName() const { return m_name; }
};
} // namespace vl
