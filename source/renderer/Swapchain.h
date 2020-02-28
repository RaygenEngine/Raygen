#pragma once

#include "renderer/LogicalDevice.h"

#include <vulkan/vulkan.hpp>


struct Swapchain {

	vk::UniqueSwapchainKHR handle;

	vk::Format imageFormat;
	vk::Extent2D extent;

	std::vector<vk::Image> images;
	std::vector<vk::UniqueImageView> imageViews;

	vk::UniqueImage depthImage;
	vk::UniqueDeviceMemory depthImageMemory;
	vk::UniqueImageView depthImageView;

	Swapchain(LogicalDevice* ld, vk::SurfaceKHR surface);
};
