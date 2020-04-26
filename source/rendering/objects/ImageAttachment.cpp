#include "pch.h"
#include "ImageAttachment.h"

#include "rendering/Device.h"
#include "rendering/VulkanUtl.h"

namespace vl {

ImageAttachment::ImageAttachment(const std::string& name, uint32 width, uint32 height, vk::Format format,
	vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, bool isDepth)
	: Image(vk::ImageType::e2D, { width, height, 1u }, 1u, 1u, format, vk::ImageTiling::eOptimal, initalLayout, usage,
		vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, {}, properties)
	, m_name(name)
	, m_isDepth(isDepth)
{
	auto testComp = m_imageInfo.extent.width >= 1 && m_imageInfo.extent.height >= 1 && m_imageInfo.extent.depth == 1
					&& m_imageInfo.arrayLayers >= 1 && m_imageInfo.samples == vk::SampleCountFlagBits::e1;

	CLOG_ABORT(!testComp, "Could not create view");

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(m_handle.get()) //
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(m_imageInfo.format);
	viewInfo.subresourceRange
		.setAspectMask(GetAspectMask(m_imageInfo)) //
		.setBaseMipLevel(0u)
		.setLevelCount(1u)
		.setBaseArrayLayer(0u)
		.setLayerCount(1u);

	m_view = Device->createImageViewUnique(viewInfo);
}
} // namespace vl