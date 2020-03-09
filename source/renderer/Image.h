#pragma once
#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"

#include <vulkan/vulkan.hpp>

class Image {
	// keep createInfo for compatibility testing
	vk::ImageCreateInfo m_imageInfo;
	vk::ImageLayout m_currentLayout;


	vk::UniqueImage m_handle;
	vk::UniqueDeviceMemory m_memory;

	void Init(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
		vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage,
		vk::SampleCountFlagBits samples, vk::SharingMode sharingMode, vk::MemoryPropertyFlags properties);

public:
	Image(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
		vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage,
		vk::SampleCountFlagBits samples, vk::SharingMode sharingMode, vk::MemoryPropertyFlags properties);

	Image(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties);

	// Viewers
	// see for compatibily
	// https://www.khronos.org/registry/vulkan/specs/1.2-extensions/html/vkspec.html#resources-image-views-compatibility
	// Getters are in form of GetImageView<Dim>_<Arrayed>_<Multisample>
	// Getters are also expected to test for view - image compatibility
	vk::UniqueImageView RequestImageView2D_0_0();

	// Transitions
	void TransitionToLayout(vk::ImageLayout newLayout);

	void CopyBufferToImage(vk::Buffer buffer);
};
