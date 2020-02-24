#include "pch/pch.h"

#include "renderer/SwapChain.h"

#include "system/Engine.h"

#include "system/Logger.h"

#include <glfw/glfw3.h>

namespace {
struct SwapChainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};

vk::SurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats)
{
	// search for req format
	for (const auto& availableFormat : availableFormats) {
		if (availableFormat.format == vk::Format::eB8G8R8A8Unorm
			&& availableFormat.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
			return availableFormat;
		}
	}

	// if search fails just return the first available
	return availableFormats[0];
}

vk::PresentModeKHR ChooseSwapPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes)
{
	for (const auto& availablePresentMode : availablePresentModes) {
		// if available, triple buffering approach
		if (availablePresentMode == vk::PresentModeKHR::eMailbox) {
			return availablePresentMode;
		}
	}

	return vk::PresentModeKHR::eFifo;
}

vk::Extent2D ChooseSwapExtent(const vk::SurfaceCapabilitiesKHR& capabilities)
{
	if (capabilities.currentExtent.width != UINT32_MAX) {
		return capabilities.currentExtent;
	}
	else {
		int32 height;
		int32 width;
		glfwGetWindowSize(Engine::GetMainWindow(), &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(
			capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(
			capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
} // namespace


Swapchain::Swapchain(DeviceWrapper& device, vk::SurfaceKHR surface)
	: m_assocDevice(device)
	, m_assocSurface(surface)
{
	auto physicalDevice = m_assocDevice.GetPhysicalDevice();

	auto details = physicalDevice.GetSwapchainSupportDetails(m_assocSurface);

	vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.formats);
	vk::PresentModeKHR presentMode = ChooseSwapPresentMode(details.presentModes);
	vk::Extent2D extent = ChooseSwapExtent(details.capabilities);

	uint32 imageCount = details.capabilities.minImageCount + 1;

	if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
		imageCount = details.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo.setSurface(surface)
		.setMinImageCount(imageCount)
		.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	auto graphicsQueueFamily = m_assocDevice.GetGraphicsQueue().familyIndex;
	auto presentQueueFamily = m_assocDevice.GetPresentQueue().familyIndex;

	uint32 queueFamilyIndices[] = { graphicsQueueFamily, presentQueueFamily };
	if (graphicsQueueFamily != presentQueueFamily) {
		createInfo.setImageSharingMode(vk::SharingMode::eConcurrent)
			.setQueueFamilyIndexCount(2)
			.setPQueueFamilyIndices(queueFamilyIndices);
	}
	else {
		createInfo.setImageSharingMode(vk::SharingMode::eExclusive)
			.setQueueFamilyIndexCount(0)
			.setPQueueFamilyIndices(nullptr);
	}

	createInfo.setPreTransform(details.capabilities.currentTransform)
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(presentMode)
		.setClipped(VK_TRUE)
		.setOldSwapchain(nullptr);


	m_handle = m_assocDevice->createSwapchainKHRUnique(createInfo);
	m_images = m_assocDevice->getSwapchainImagesKHR(m_handle.get());

	// Store swap chain image format and extent
	m_imageFormat = surfaceFormat.format;
	m_extent = extent;


	// views
	for (const auto& img : m_images) {

		vk::ImageViewCreateInfo viewInfo{};
		viewInfo.setImage(img).setViewType(vk::ImageViewType::e2D).setFormat(m_imageFormat);
		viewInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
			.setBaseMipLevel(0u)
			.setLevelCount(1u)
			.setBaseArrayLayer(0u)
			.setLayerCount(1u);

		m_imageViews.emplace_back(device->createImageViewUnique(viewInfo));
	}

	// render pass

	vk::AttachmentDescription colorAttachment{};
	colorAttachment.setFormat(m_imageFormat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);


	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.setAttachment(0);
	colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentDescription depthAttachment{};
	depthAttachment.setFormat(physicalDevice.FindDepthFormat())
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef.setAttachment(1);
	depthAttachmentRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass.setColorAttachmentCount(1)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef)
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.setAttachmentCount(static_cast<uint32>(attachments.size()))
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	m_renderPass = device->createRenderPassUnique(renderPassInfo);

	// depth buffer
	vk::Format depthFormat = physicalDevice.FindDepthFormat();
	m_assocDevice.CreateImage(extent.width, extent.height, depthFormat, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal, m_depthImage,
		m_depthImageMemory);

	// We don't need to explicitly transition the layout of the image to a depth attachment because we'll take care of
	// this in the render pass.
	m_assocDevice.TransitionImageLayout(
		m_depthImage.get(), depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo.setImage(m_depthImage.get()).setViewType(vk::ImageViewType::e2D).setFormat(depthFormat);
	viewInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	m_depthImageView = m_assocDevice->createImageViewUnique(viewInfo);

	// framebuffers
	for (auto& imgv : m_imageViews) {
		std::array<vk::ImageView, 2> attachments = { imgv.get(), m_depthImageView.get() };
		vk::FramebufferCreateInfo createInfo{};
		createInfo.setRenderPass(m_renderPass.get())
			.setAttachmentCount(static_cast<uint32>(attachments.size()))
			.setPAttachments(attachments.data())
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1);

		m_framebuffers.emplace_back(device->createFramebufferUnique(createInfo));
	}
}
