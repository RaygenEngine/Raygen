#include "pch.h"
#include "RSwapchain.h"

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

		vk::Extent2D actualExtent = { static_cast<uint32_t>(width), static_cast<uint32_t>(height) };

		actualExtent.width = std::max(
			capabilities.minImageExtent.width, std::min(capabilities.maxImageExtent.width, actualExtent.width));
		actualExtent.height = std::max(
			capabilities.minImageExtent.height, std::min(capabilities.maxImageExtent.height, actualExtent.height));

		return actualExtent;
	}
}
} // namespace

namespace vl {
RSwapchain::RSwapchain(vk::SurfaceKHR surface)
{
	auto details = Device->GetSwapchainSupportDetails();

	vk::SurfaceFormatKHR surfaceFormat = ChooseSwapSurfaceFormat(details.formats);
	vk::PresentModeKHR presentMode = ChooseSwapPresentMode(details.presentModes);
	extent = ChooseSwapExtent(details.capabilities);
	imageFormat = surfaceFormat.format;

	imageCount = std::max(details.capabilities.minImageCount, 3u);

	if (details.capabilities.maxImageCount > 0u && imageCount > details.capabilities.maxImageCount) {
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

	// graphics and present families for swapchain image concurrency
	auto mainQueueFamily = Device->mainQueue.familyIndex;
	auto presentQueueFamily = Device->presentQueue.familyIndex;

	uint32 queueFamilyIndices[] = { mainQueueFamily, presentQueueFamily };
	if (mainQueueFamily != presentQueueFamily) {
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

	vk::SwapchainKHR::operator=(Device->createSwapchainKHR(createInfo));

	images = Device->getSwapchainImagesKHR(*this);

	InitRenderPass();
	InitImageViews();
	InitFrameBuffers();
}

RSwapchain::~RSwapchain()
{
	Device->destroySwapchainKHR(*this);
}

void RSwapchain::InitRenderPass()
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
		.setColorAttachmentCount(1u)
		.setPColorAttachments(&colorAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachmentCount(1u) //
		.setPAttachments(&colorAttachment)
		.setSubpassCount(1u)
		.setPSubpasses(&subpass)
		.setDependencyCount(1u)
		.setPDependencies(&dependency);

	renderPass = Device->createRenderPassUnique(renderPassInfo);
}

void RSwapchain::InitImageViews()
{
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
}

void RSwapchain::InitFrameBuffers()
{
	framebuffers.clear();
	framebuffers.resize(images.size());

	for (auto i = 0; i < images.size(); ++i) {

		vk::FramebufferCreateInfo createInfo{};
		createInfo
			.setRenderPass(renderPass.get()) //
			.setAttachmentCount(1u)
			.setPAttachments(&imageViews[i].get())
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1u);

		framebuffers[i] = Device->createFramebufferUnique(createInfo);
	}
}
} // namespace vl
