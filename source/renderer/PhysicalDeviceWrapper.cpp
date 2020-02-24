#include "pch/pch.h"

#include "renderer/PhysicalDeviceWrapper.h"
#include "renderer/DeviceWrapper.h"

#include "system/Logger.h"

#include <set>


void PhysicalDeviceWrapper::Init(vk::PhysicalDevice handle, vk::SurfaceKHR surface)
{
	m_vkHandle = handle;

	std::vector<vk::QueueFamilyProperties> queueFamilyProperties = m_vkHandle.getQueueFamilyProperties();

	uint32 i = 0;
	for (const auto& qFP : queueFamilyProperties) {

		QueueFamily queueFamily{};
		queueFamily.index = i;
		queueFamily.props = qFP;
		queueFamily.rating += qFP.queueCount / 4.f;

		// if queue supports presentation
		auto supportsPresent = m_vkHandle.getSurfaceSupportKHR(i, surface);


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

SwapchainSupportDetails PhysicalDeviceWrapper::GetSwapchainSupportDetails(vk::SurfaceKHR surface)
{
	SwapchainSupportDetails details;

	details.capabilities = m_vkHandle.getSurfaceCapabilitiesKHR(surface);
	details.formats = m_vkHandle.getSurfaceFormatsKHR(surface);
	details.presentModes = m_vkHandle.getSurfacePresentModesKHR(surface);

	return details;
}

uint32 PhysicalDeviceWrapper::FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties)
{
	vk::PhysicalDeviceMemoryProperties memProperties = m_vkHandle.getMemoryProperties();

	for (uint32 i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LOG_ABORT("Failed to find suitable memory type!");
}

vk::Format PhysicalDeviceWrapper::FindSupportedFormat(
	const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features)
{
	for (auto format : candidates) {
		vk::FormatProperties props = m_vkHandle.getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		else if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	LOG_ABORT("Failed to find supported format!");
}

vk::Format PhysicalDeviceWrapper::FindDepthFormat()
{
	return FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}
