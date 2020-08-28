#pragma once

namespace vl {
struct RSwapchain {

	uint32 imageCount;

	vk::Format imageFormat;
	vk::Extent2D extent;

	RSwapchain(vk::SurfaceKHR surface);

	[[nodiscard]] vk::SwapchainKHR handle() const { return uHandle.get(); }
	[[nodiscard]] vk::RenderPass renderPass() const { return uRenderPass.get(); }
	[[nodiscard]] vk::Framebuffer framebuffer(uint32 imageIndex) const { return uFramebuffers.at(imageIndex).get(); };

private:
	void InitRenderPass();
	void InitImageViews();
	void InitFrameBuffers();

	vk::UniqueSwapchainKHR uHandle;
	vk::UniqueRenderPass uRenderPass;

	std::vector<vk::Image> images;
	std::vector<vk::UniqueImageView> uImageViews;

	std::vector<vk::UniqueFramebuffer> uFramebuffers;
};
} // namespace vl
