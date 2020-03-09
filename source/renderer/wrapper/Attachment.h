#pragma once
#include "renderer/wrapper/Image.h"

#include <vulkan/vulkan.hpp>

struct Attachment {

	std::unique_ptr<Image> image;
	vk::UniqueImageView view;

	Attachment(uint32 width, uint32 height, vk::Format format, vk::ImageUsageFlags usage);
};
