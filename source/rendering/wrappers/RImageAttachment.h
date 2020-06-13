#pragma once
#include "rendering/wrappers/RImage.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class RImageAttachment : public RImage {

	std::string m_name;
	bool m_isDepth;

public:
	RImageAttachment(const std::string& name, uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);

	[[nodiscard]] const std::string& GetName() const { return m_name; }
	[[nodiscard]] bool IsDepth() const { return m_isDepth; }
};
} // namespace vl
