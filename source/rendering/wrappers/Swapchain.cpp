#include "pch.h"
#include "Swapchain.h"

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
		.setImageUsage(vk::ImageUsageFlagBits::eColorAttachment)
		.setPreTransform(details.capabilities.currentTransform) //
		.setCompositeAlpha(vk::CompositeAlphaFlagBitsKHR::eOpaque)
		.setPresentMode(presentMode)
		.setClipped(VK_TRUE);

	// graphics and present families for swapchain image concurrency
	auto mainQueueFamily = Device->graphicsQueue.familyIndex;
	auto presentQueueFamily = Device->presentQueue.familyIndex;

	if (mainQueueFamily != presentQueueFamily) {
		createInfo
			.setImageSharingMode(vk::SharingMode::eConcurrent) //
			.setQueueFamilyIndices(std::array{ mainQueueFamily, presentQueueFamily });
	}
	else {
		createInfo.setImageSharingMode(vk::SharingMode::eExclusive);
		// CHECK
		//.setQueueFamilyIndexCount(0u)
		//.setPQueueFamilyIndices(nullptr);
	}

	uHandle = Device->createSwapchainKHRUnique(createInfo);

	images = Device->getSwapchainImagesKHR(uHandle.get());

	InitRenderPass();
	InitImageViews();
	InitFrameBuffers();
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
		.setColorAttachments(colorAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL)   //
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstSubpass(0u)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachments(colorAttachment) //
		.setSubpasses(subpass)
		.setDependencies(dependency);

	uRenderPass = Device->createRenderPassUnique(renderPassInfo);
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

		uImageViews.emplace_back(Device->createImageViewUnique(viewInfo));
	}
}

void RSwapchain::InitFrameBuffers()
{
	uFramebuffers.clear();
	uFramebuffers.resize(imageCount);

	for (uint32 i = 0; i < imageCount; ++i) {
		vk::FramebufferCreateInfo createInfo{};
		createInfo
			.setRenderPass(uRenderPass.get()) //
			.setAttachments(uImageViews[i].get())
			.setWidth(extent.width)
			.setHeight(extent.height)
			.setLayers(1u);

		uFramebuffers[i] = Device->createFramebufferUnique(createInfo);
	}
}
} // namespace vl
