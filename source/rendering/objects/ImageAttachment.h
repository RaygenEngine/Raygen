#pragma once
#include "rendering/objects/Image.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class ImageAttachment : public Image {

	std::string m_name;
	bool m_isDepth;

public:
	// WIP: check depth from format
	ImageAttachment(const std::string& name, uint32 width, uint32 height, vk::Format format,
		vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, bool isDepth);

	[[nodiscard]] std::string GetName() const { return m_name; }
	[[nodiscard]] bool IsDepth() const { return m_isDepth; }
};
} // namespace vl
