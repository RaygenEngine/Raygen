#pragma once

#include "renderer/renderers/vulkan/Device.h"

#include <vulkan/vulkan.hpp>

namespace vulkan {
class Swapchain {

	vk::SwapchainKHR m_handle;

	vk::Format m_imageFormat;
	vk::Extent2D m_extent;

	std::vector<vk::Image> m_images;
	std::vector<vk::ImageView> m_imageViews;

	vk::RenderPass m_renderPass;

	std::vector<vk::Framebuffer> m_framebuffers;

	Device* m_assocDevice;
	vk::SurfaceKHR m_assocSurface;

	// WIP: depth image
	vk::Image depthImage;
	vk::DeviceMemory depthImageMemory;
	vk::ImageView depthImageView;


public:
	Swapchain(Device* device, vk::SurfaceKHR surface);

	vk::SwapchainKHR Get() const { return m_handle; }
	vk::Format GetImageFormat() const { return m_imageFormat; }
	vk::Extent2D GetExtent() const { return m_extent; }
	vk::RenderPass GetRenderPass() const { return m_renderPass; }
	std::vector<vk::Image> GetImages() const { return m_images; }
	std::vector<vk::ImageView> GetImageViews() const { return m_imageViews; }
	std::vector<vk::Framebuffer> GetFramebuffers() const { return m_framebuffers; }

	uint32 GetImageCount() const { return static_cast<uint32>(m_images.size()); }
};
} // namespace vulkan
