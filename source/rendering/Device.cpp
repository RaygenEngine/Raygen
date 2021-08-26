#include "Device.h"

#include "rendering/VkCoreIncludes.h"
#include "rendering/VulkanLoader.h"

#include <set>

namespace vl {
Device_::Device_(RPhysicalDevice& pd)
	: pd(pd)
{
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32> uniqueQueueFamilies = {
		pd.graphicsFamily.index,
		pd.dmaFamily.index,
		pd.computeFamily.index,
		pd.presentFamily.index,
	};

	std::array qps{ 1.0f };
	vk::DeviceQueueCreateInfo createInfo{};
	for (uint32 queueFamily : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo createInfo{};
		createInfo
			.setQueueFamilyIndex(queueFamily) //
			.setQueuePriorities(qps);
		queueCreateInfos.push_back(createInfo);
	}

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo
		.setQueueCreateInfos(queueCreateInfos) //
		.setPEnabledExtensionNames(pd.extensions)
		.setPNext(&pd.featuresChain.get<vk::PhysicalDeviceFeatures2>());

	vk::Device::operator=(pd.createDevice(deviceCreateInfo));
	VulkanLoader::InitLoaderWithDevice(*this);
}

uint32 Device_::FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties) const
{
	vk::PhysicalDeviceMemoryProperties memProperties = pd.getMemoryProperties();

	for (uint32 i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LOG_ABORT("Failed to find suitable memory type!");
}

vk::Format Device_::FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, //
	const vk::FormatFeatureFlags features) const
{
	for (auto format : candidates) {
		vk::FormatProperties props = pd.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	LOG_ABORT("Failed to find supported format!");
}

vk::Format Device_::FindDepthFormat() const
{
	return FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format Device_::FindDepthStencilFormat() const
{
	return FindSupportedFormat(
		{ vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint, vk::Format::eD16UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

vk::Format Device_::FindStencilFormat() const
{
	return FindSupportedFormat(
		{ vk::Format::eS8Uint }, vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}


SwapchainSupportDetails Device_::GetSwapchainSupportDetails() const
{
	SwapchainSupportDetails ssDetails;

	ssDetails.capabilities = pd.getSurfaceCapabilitiesKHR(pd.surface);
	ssDetails.formats = pd.getSurfaceFormatsKHR(pd.surface);
	ssDetails.presentModes = pd.getSurfacePresentModesKHR(pd.surface);
	return ssDetails;
}

Device_::~Device_()
{
	destroy();
}

CmdPoolManager_::CmdPoolManager_()
	// CHECK: yikes
	: graphicsQueue(Device->pd.graphicsFamily)
	, dmaQueue(Device->pd.dmaFamily)
	, computeQueue(Device->pd.computeFamily)
	, presentQueue(Device->pd.presentFamily)
	, graphicsCmdPool(graphicsQueue)
	, dmaCmdPool(dmaQueue)
	, computeCmdPool(computeQueue)
{
	DEBUG_NAME(graphicsQueue, "Graphics Queue");
	DEBUG_NAME(dmaQueue, "Dma Queue");
	DEBUG_NAME(computeQueue, "Compute Queue");
	DEBUG_NAME(presentQueue, "Present Queue");
}
} // namespace vl
