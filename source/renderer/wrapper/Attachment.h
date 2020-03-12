#pragma once
#include "renderer/wrapper/Image.h"

#include <vulkan/vulkan.hpp>
#include <optional>

struct Attachment {

	std::unique_ptr<Image> image;
	vk::UniqueImageView view;

	std::optional<vk::DescriptorSet> debugDescriptorSet;


	Attachment(
		uint32 width, uint32 height, vk::Format format, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage);


	vk::DescriptorSet GetDebugDescriptor();
};
