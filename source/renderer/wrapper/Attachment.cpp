#include "pch.h"
#include "renderer/wrapper/Attachment.h"

Attachment::Attachment(uint32 width, uint32 height, vk::Format format, vk::ImageUsageFlags usage)
{
	image = std::make_unique<Image>(
		width, height, format, vk::ImageTiling::eOptimal, usage, vk::MemoryPropertyFlagBits::eDeviceLocal);

	view = image->RequestImageView2D_0_0();
}
