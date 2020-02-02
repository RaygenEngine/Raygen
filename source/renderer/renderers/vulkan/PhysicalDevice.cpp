#include "pch/pch.h"

#include "renderer/renderers/vulkan/PhysicalDevice.h"
#include "renderer/renderers/vulkan/Device.h"

#include "system/Logger.h"

#include <set>

namespace vulkan {

// WIP:
std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME };

QueueFamily GetQueueWithBestRating(const std::vector<QueueFamily>& queueFamilies)
{
	auto it = std::max_element(
		queueFamilies.begin(), queueFamilies.end(), [](QueueFamily a, QueueFamily b) { return a.rating < b.rating; });

	return { *it };
}

PhysicalDevice::PhysicalDevice(vk::PhysicalDevice handle, vk::SurfaceKHR surface)
	: vk::PhysicalDevice(handle)
{

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = getQueueFamilyProperties();

	uint32 i = 0;
	for (const auto& qFP : queueFamilyProperties) {

		QueueFamily queueFamily{};
		queueFamily.familyIndex = i;
		queueFamily.props = qFP;
		queueFamily.rating += qFP.queueCount / 4.f;

		// if queue supports presentation
		auto supportsPresent = getSurfaceSupportKHR(i, surface);


		// WIP: the more functionality a queue supports the lesser the score
		queueFamily.rating /= float(((4 * bool(qFP.queueFlags & vk::QueueFlagBits::eGraphics))
									 + (4 * bool(qFP.queueFlags & vk::QueueFlagBits::eCompute))
									 + bool(qFP.queueFlags & vk::QueueFlagBits::eTransfer) + supportsPresent));

		if (qFP.queueFlags & vk::QueueFlagBits::eGraphics) {
			m_graphicsFamilies.push_back(queueFamily);
		}

		if (qFP.queueFlags & vk::QueueFlagBits::eCompute) {
			m_computeFamilies.push_back(queueFamily);
		}

		if (qFP.queueFlags & vk::QueueFlagBits::eTransfer) {
			m_transferFamilies.push_back(queueFamily);
		}

		if (supportsPresent) {
			m_presentFamilies.push_back(queueFamily);
		}

		i++;
	}

	// TODO: score device based on eg. NV_raytracing, queues: eg. missing graphics queue, deticated transfer queue,
	// etc)
	// If device isn't capable for the required rendering rating = 0;
	// auto extensionProperties = device.enumerateDeviceExtensionProperties();
	// auto layerProperties = device.enumerateDeviceLayerProperties();

	// auto queueFamilyProps = device.getQueueFamilyProperties();

	// for (auto qF : queueFamilyProps) {
	//	if (qF.queueFlags & vk::QueueFlagBits::eGraphics) {
	//	}
	//}

	// Rate based on those functions
	// indices.IsComplete() && CheckNeededDeviceProperties(device) && CheckIfDeviceExtensionsAreAvailable(device)
	//	&& CheckSwapChainSupport(device, surface);

	// WIP:

	// TODO: this device must support presentation
	if (m_presentFamilies.empty()) {
		m_rating = 0;
	}

	m_rating = 1u;
}

QueueFamily PhysicalDevice::GetBestGraphicsFamily() const
{
	return GetQueueWithBestRating(m_graphicsFamilies);
}

QueueFamily PhysicalDevice::GetBestTransferFamily() const
{
	return GetQueueWithBestRating(m_transferFamilies);
}

QueueFamily PhysicalDevice::GetBestComputeFamily() const
{
	return GetQueueWithBestRating(m_computeFamilies);
}

QueueFamily PhysicalDevice::GetBestPresentFamily() const
{
	return GetQueueWithBestRating(m_presentFamilies);
}

SwapchainSupportDetails PhysicalDevice::GetSwapchainSupportDetails(vk::SurfaceKHR surface)
{
	SwapchainSupportDetails details;

	details.capabilities = getSurfaceCapabilitiesKHR(surface);
	details.formats = getSurfaceFormatsKHR(surface);
	details.presentModes = getSurfacePresentModesKHR(surface);

	return details;
}

std::unique_ptr<Device> PhysicalDevice::RequestLogicalDevice()
{
	auto graphicsQueueFamily = GetBestGraphicsFamily();
	auto transferQueueFamily = GetBestTransferFamily();
	auto presentQueueFamily = GetBestPresentFamily();

	// Get device's presentation queue
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32> uniqueQueueFamilies
		= { graphicsQueueFamily.familyIndex, transferQueueFamily.familyIndex, presentQueueFamily.familyIndex };

	float qp1 = 1.0f;
	for (uint32 queueFamily : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo createInfo{};
		createInfo.setQueueFamilyIndex(queueFamily).setQueueCount(1).setPQueuePriorities(&qp1);
		queueCreateInfos.push_back(createInfo);
	}

	// TODO: (get from assoc)
	vk::PhysicalDeviceFeatures deviceFeatures = {};

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data())
		.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
		.setPEnabledFeatures(&deviceFeatures)
		.setPpEnabledExtensionNames(deviceExtensions.data())
		.setEnabledExtensionCount(static_cast<uint32>(deviceExtensions.size()))
		.setEnabledLayerCount(0);

	auto handle = createDevice(deviceCreateInfo);

	return std::make_unique<Device>(handle, this);
}

uint32 PhysicalDevice::FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = getMemoryProperties();

	for (uint32 i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LOG_ABORT("Failed to find suitable memory type!");
}

vk::Format PhysicalDevice::FindSupportedFormat(
	const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for (auto format : candidates) {
		vk::FormatProperties props = getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	LOG_ABORT("Failed to find supported format!");
}

vk::Format PhysicalDevice::FindDepthFormat()
{
	return FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

} // namespace vulkan
