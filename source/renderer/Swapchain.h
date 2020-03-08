#pragma once

#include "renderer/LogicalDevice.h"

#include <vulkan/vulkan.hpp>


struct Swapchain {

	vk::UniqueSwapchainKHR handle;

	vk::Format imageFormat;
	vk::Extent2D extent;

	std::vector<vk::Image> images;
	std::vector<vk::UniqueImageView> imageViews;

	std::vector<vk::UniqueFramebuffer> framebuffers;
	vk::UniqueRenderPass renderPass;

	Swapchain(LogicalDevice* ld, vk::SurfaceKHR surface);

private:
	void InitRenderPass();
	void InitFrameBuffers();
};
