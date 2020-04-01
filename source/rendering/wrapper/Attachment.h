#pragma once
#include "rendering/wrapper/ImageObj.h"

#include <vulkan/vulkan.hpp>
#include <optional>

namespace vl {
struct Attachment {

	UniquePtr<ImageObj> image;
	vk::UniqueImageView view;

	vk::Format format;

	std::optional<vk::DescriptorSet> debugDescriptorSet;


	Attachment(
		uint32 width, uint32 height, vk::Format format, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage);


	vk::DescriptorSet GetDebugDescriptor();
};

} // namespace vl
