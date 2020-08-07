#pragma once
#include <vulkan/vulkan.hpp>

namespace vl {
inline struct Swapchain_ : public vk::SwapchainKHR {

	uint32 imageCount;

	vk::Format imageFormat;
	vk::Extent2D extent;

	std::vector<vk::Image> images;
	std::vector<vk::UniqueImageView> imageViews;

	std::vector<vk::UniqueFramebuffer> framebuffers;
	
	vk::UniqueRenderPass renderPass;

	Swapchain_(vk::SurfaceKHR surface);
	~Swapchain_();

private:
	void InitRenderPass();
	void InitImageViews();
	void InitFrameBuffers();

} * Swapchain{};
} // namespace vl
