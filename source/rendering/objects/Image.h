#pragma once
#include "rendering/objects/Buffer.h"
#include "rendering/objects/DescriptorLayout.h"

#include <vulkan/vulkan.hpp>
#include <optional>

// DOC: child classes should create the appropriate view
// TODO: find a way to avoid the device wait idles in this class
namespace vl {
class Image {
	std::optional<vk::DescriptorSet> m_debugDescriptorSet;

protected:
	vk::ImageCreateInfo m_imageInfo;

	vk::UniqueImage m_handle;
	vk::UniqueDeviceMemory m_memory;

	vk::UniqueImageView m_view;


	Image(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
		vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage,
		vk::SampleCountFlagBits samples, vk::SharingMode sharingMode, vk::ImageCreateFlags flags,
		vk::MemoryPropertyFlags properties);

public:
	// Blocking transition to layout
	void BlockingTransitionToLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

	void CopyBufferToImage(const RawBuffer& buffer);

	// WIP: virtual (split image classes correctly...)
	virtual void GenerateMipmapsAndTransitionEach(vk::ImageLayout oldLayout, vk::ImageLayout finalLayout);

	// Affects all mips and array elements
	vk::ImageMemoryBarrier CreateTransitionBarrier(
		vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32 baseMipLevel = 0u, uint32 baseArrayLevel = 0u);

	operator vk::Image() const noexcept { return m_handle.get(); }
	[[nodiscard]] vk::DeviceMemory GetMemory() const { return m_memory.get(); }
	[[nodiscard]] vk::ImageView GetView() const { return m_view.get(); }
	[[nodiscard]] vk::Format GetFormat() const { return m_imageInfo.format; }
	[[nodiscard]] vk::Extent3D GetExtent3D() const { return m_imageInfo.extent; }
	[[nodiscard]] vk::Extent2D GetExtent2D() const { return { m_imageInfo.extent.width, m_imageInfo.extent.height }; }

	[[nodiscard]] vk::DescriptorSet GetDebugDescriptor();

	virtual ~Image() = default;
};
} // namespace vl