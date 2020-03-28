#include "pch.h"
#include "Swapchain.h"

#include "engine/Engine.h"
#include "engine/Logger.h"
#include "platform/Platform.h"
#include "rendering/Device.h"

#include <glfw/glfw3.h>

namespace {
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
		glfwGetWindowSize(Platform::GetMainHandle(), &width, &height);

		VkExtent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(
			capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(
			capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
} // namespace

namespace vl {

Swapchain::~Swapchain() {}

Swapchain::Swapchain(vk::SurfaceKHR surface)
{
	auto pd = Device->pd;

	auto details = pd->GetSwapchainSupportDetails();

	vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.formats);
	vk::PresentModeKHR presentMode = ChooseSwapPresentMode(details.presentModes);
	extent = ChooseSwapExtent(details.capabilities);

	uint32 imageCount = details.capabilities.minImageCount;

	if (details.capabilities.maxImageCount > 0 && imageCount > details.capabilities.maxImageCount) {
		imageCount = details.capabilities.maxImageCount;
	}

	vk::SwapchainCreateInfoKHR createInfo{};
	createInfo
		.setSurface(surface) //
		.setMinImageCount(imageCount)

		.setImageFormat(surfaceFormat.format)
		.setImageColorSpace(surfaceFormat.colorSpace)
		.setImageExtent(extent)
		.setImageArrayLayers(1u)
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment);

	auto graphicsQueueFamily = Device->graphicsQueue.familyIndex;
	auto presentQueueFamily = Device->presentQueue.familyIndex;

	uint32 queueFamilyIndices[] = { graphicsQueueFamily, presentQueueFamily };
	if (graphicsQueueFamily != presentQueueFamily) {
		createInfo
			.setImageSharingMode(vk::SharingMode::eConcurrent) //
			.setQueueFamilyIndexCount(2u)
			.setPQueueFamilyIndices(queueFamilyIndices);
	}
	else {
		createInfo
			.setImageSharingMode(vk::SharingMode::eExclusive) //
			.setQueueFamilyIndexCount(0u)
			.setPQueueFamilyIndices(nullptr);
	}

	createInfo
		.setPreTransform(details.capabilities.currentTransform) //
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(presentMode)
		.setClipped(VK_TRUE)
		.setOldSwapchain(nullptr);


	handle = Device->createSwapchainKHRUnique(createInfo);
	images = Device->getSwapchainImagesKHR(handle.get());

	// Store swap chain image format and extent
	imageFormat = surfaceFormat.format;


	// views
	for (const auto& img : images) {

		vk::ImageViewCreateInfo viewInfo{};
		viewInfo
			.setImage(img) //
			.setViewType(vk::ImageViewType::e2D)
			.setFormat(imageFormat);
		viewInfo.subresourceRange
			.setAspectMask(vk::ImageAspectFlagBits::eColor) //
			.setBaseMipLevel(0u)
			.setLevelCount(1u)
			.setBaseArrayLayer(0u)
			.setLayerCount(1u);

		imageViews.emplace_back(Device->createImageViewUnique(viewInfo));
	}

	InitRenderPass();
	InitFrameBuffers();
}

void Swapchain::InitRenderPass()
{
	vk::AttachmentDescription colorAttachment{};
	colorAttachment
		.setFormat(imageFormat) //
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);


	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::array attachments{ colorAttachment };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachmentCount(static_cast<uint32>(attachments.size())) //
		.setPAttachments(attachments.data())
		.setSubpassCount(1u)
		.setPSubpasses(&subpass)
		.setDependencyCount(1u)
		.setPDependencies(&dependency);

	renderPass = Device->createRenderPassUnique(renderPassInfo);
}

void Swapchain::InitFrameBuffers()
{
	framebuffers.clear();
	framebuffers.resize(images.size());
	// framebuffers
	for (auto i = 0; i < images.size(); ++i) {
		std::array attachments{ imageViews[i].get() };
		vk::FramebufferCreateInfo createInfo{};
		createInfo
			.setRenderPass(renderPass.get()) //
			.setAttachmentCount(static_cast<uint32>(attachments.size()))
			.setPAttachments(attachments.data())
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1u);

		framebuffers[i] = Device->createFramebufferUnique(createInfo);
	}
}
} // namespace vl
