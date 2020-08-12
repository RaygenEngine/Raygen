#pragma once

namespace vl {
struct RSwapchain : vk::SwapchainKHR {

	uint32 imageCount;

	vk::Format imageFormat;
	vk::Extent2D extent;

	std::vector<vk::Image> images;
	std::vector<vk::UniqueImageView> imageViews;

	std::vector<vk::UniqueFramebuffer> framebuffers;

	vk::UniqueRenderPass renderPass;

	RSwapchain(vk::SurfaceKHR surface);
	~RSwapchain();

private:
	void InitRenderPass();
	void InitImageViews();
	void InitFrameBuffers();
};
} // namespace vl
