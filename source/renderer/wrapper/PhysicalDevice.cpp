#include "pch.h"
#include "renderer/wrapper/PhysicalDevice.h"

#include "engine/Logger.h"

#include <set>

PhysicalDevice::PhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR inSurface)
	: vk::PhysicalDevice(vkHandle)
	, surface(inSurface)
{
	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = getQueueFamilyProperties();

	uint32 i = 0;
	for (const auto& qFP : queueFamilyProperties) {

		QueueFamily queueFamily{};
		queueFamily.index = i;
		queueFamily.props = qFP;
		queueFamily.rating += qFP.queueCount / 4.f;

		// if queue supports presentation
		auto supportsPresent = getSurfaceSupportKHR(i, surface);


		// CHECK: the more functionality a queue supports the lesser the score
		queueFamily.rating /= float(((4 * bool(qFP.queueFlags & vk::QueueFlagBits::eGraphics))
									 + (4 * bool(qFP.queueFlags & vk::QueueFlagBits::eCompute))
									 + bool(qFP.queueFlags & vk::QueueFlagBits::eTransfer) + supportsPresent));

		if (qFP.queueFlags & vk::QueueFlagBits::eGraphics) {
			graphicsFamilies.push_back(queueFamily);
		}

		if (qFP.queueFlags & vk::QueueFlagBits::eCompute) {
			computeFamilies.push_back(queueFamily);
		}

		if (qFP.queueFlags & vk::QueueFlagBits::eTransfer) {
			transferFamilies.push_back(queueFamily);
		}

		if (supportsPresent) {
			presentFamilies.push_back(queueFamily);
		}

		i++;
	}

	// CHECK: score device based on eg. NV_raytracing, queues: eg. missing graphics queue, deticated transfer queue,
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

	// CHECK: this device must support presentation
	if (presentFamilies.empty()) {
		rating = 0.f;
	}

	rating = 1.f;

	// specific surface suppor[t details
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

SwapchainSupportDetails PhysicalDevice::GetSwapchainSupportDetails() const
{
	SwapchainSupportDetails ssDetails;

	ssDetails.capabilities = getSurfaceCapabilitiesKHR(surface);
	ssDetails.formats = getSurfaceFormatsKHR(surface);
	ssDetails.presentModes = getSurfacePresentModesKHR(surface);
	return ssDetails;
}
