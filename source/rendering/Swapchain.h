#pragma once
#include <vulkan/vulkan.hpp>

namespace vl {
inline class Swapchain_ {

	vk::UniqueSwapchainKHR m_handle;

	vk::Format m_imageFormat;
	vk::Extent2D m_extent;

	std::vector<vk::Image> m_images;
	std::vector<vk::UniqueImageView> m_imageViews;

	std::vector<vk::UniqueFramebuffer> m_framebuffers;
	vk::UniqueRenderPass m_renderPass;

	void InitRenderPass();
	void InitFrameBuffers();

public:
	Swapchain_(vk::SurfaceKHR surface);

	[[nodiscard]] uint32 GetImageCount() const { return static_cast<uint32>(m_images.size()); }
	[[nodiscard]] vk::RenderPass GetRenderPass() const { return m_renderPass.get(); }
	[[nodiscard]] vk::Extent2D GetExtent() const { return m_extent; }
	[[nodiscard]] vk::Framebuffer GetFramebuffer(uint32 index) const { return m_framebuffers[index].get(); }
	[[nodiscard]] operator vk::SwapchainKHR() const { return m_handle.get(); }

} * Swapchain{};
} // namespace vl
