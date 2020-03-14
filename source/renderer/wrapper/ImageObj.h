#pragma once
#include "renderer/wrapper/Buffer.h"

#include <vulkan/vulkan.hpp>

class ImageObj {
	// keep createInfo for compatibility testing
	vk::ImageCreateInfo m_imageInfo;


	vk::UniqueImage m_handle;
	vk::UniqueDeviceMemory m_memory;

	void Init(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
		vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage,
		vk::SampleCountFlagBits samples, vk::SharingMode sharingMode, vk::MemoryPropertyFlags properties);

public:
	ImageObj(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
		vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage,
		vk::SampleCountFlagBits samples, vk::SharingMode sharingMode, vk::MemoryPropertyFlags properties);

	ImageObj(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageLayout initalLayout,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties);

	// Viewers
	// see for compatibily
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#resources-image-views-compatibility
	// Getters are in form of GetImageView<Dim>_<Arrayed>_<Multisample>
	// Getters are also expected to test for view - image compatibility
	vk::UniqueImageView RequestImageView2D_0_0();

	// Blocking transition to layout
	void TransitionToLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

	void CopyBufferToImage(const Buffer& buffer);

	operator vk::Image() const noexcept { return m_handle.get(); }


	vk::ImageMemoryBarrier CreateTransitionBarrier(vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
};
