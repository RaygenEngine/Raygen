#pragma once
#include <vulkan/vulkan.hpp>

namespace vl {
struct Swapchain {

	vk::UniqueSwapchainKHR handle;

	vk::Format imageFormat;
	vk::Extent2D extent;

	std::vector<vk::Image> images;
	std::vector<vk::UniqueImageView> imageViews;

	std::vector<vk::UniqueFramebuffer> framebuffers;
	vk::UniqueRenderPass renderPass;

	Swapchain(vk::SurfaceKHR surface);

private:
	void InitRenderPass();
	void InitFrameBuffers();
};
} // namespace vl
