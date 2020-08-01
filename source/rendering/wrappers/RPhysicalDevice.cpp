#include "pch.h"
#include "RPhysicalDevice.h"

#include "engine/Logger.h"

namespace vl {
RPhysicalDevice::RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR inSurface)
	: vk::PhysicalDevice(vkHandle)
	, surface(inSurface)
{
	auto queueFamilyProperties = getQueueFamilyProperties();

	for (uint32 i = 0; const auto& qFP : queueFamilyProperties) {

		QueueFamily queueFamily{};
		queueFamily.index = i;
		queueFamily.props = qFP;


		// if queue supports presentation
		auto supportsPresent = getSurfaceSupportKHR(i, surface);


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


	// Scoring based on features and properties (if missing required features by the engine the rating should be set to
	// zero
	auto propertiesChain = getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesKHR>();
	auto properties = propertiesChain.get<vk::PhysicalDeviceProperties2>();
	auto rayTracingProperties = propertiesChain.get<vk::PhysicalDeviceRayTracingPropertiesKHR>();

	auto featuresChain = getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceRayTracingFeaturesKHR>();
	auto features = featuresChain.get<vk::PhysicalDeviceFeatures2>();
	auto rayTracingFeatures = featuresChain.get<vk::PhysicalDeviceRayTracingFeaturesKHR>();

	// TODO: calculate Gpu rating:
	rating = 1.f;

	// Requirements
	if (presentFamilies.empty() || !rayTracingFeatures.rayTracing || !rayTracingFeatures.rayQuery
		|| rayTracingProperties.shaderGroupHandleSize != 32u || rayTracingProperties.maxRecursionDepth < 1) {
		rating = 0.f;
	}
}

uint32 RPhysicalDevice::FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties) const
{
	vk::PhysicalDeviceMemoryProperties memProperties = getMemoryProperties();

	for (uint32 i = 0; i < memProperties.memoryTypeCount; ++i) {
		if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
			return i;
		}
	}

	LOG_ABORT("Failed to find suitable memory type!");
}

vk::Format RPhysicalDevice::FindSupportedFormat(
	const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, const vk::FormatFeatureFlags features) const
{
	for (auto format : candidates) {
		vk::FormatProperties props = getFormatProperties(format);

		if (tiling == vk::ImageTiling::eLinear && (props.linearTilingFeatures & features) == features) {
			return format;
		}
		if (tiling == vk::ImageTiling::eOptimal && (props.optimalTilingFeatures & features) == features) {
			return format;
		}
	}

	LOG_ABORT("Failed to find supported format!");
}

vk::Format RPhysicalDevice::FindDepthFormat() const
{
	return FindSupportedFormat({ vk::Format::eD32Sfloat, vk::Format::eD32SfloatS8Uint, vk::Format::eD24UnormS8Uint },
		vk::ImageTiling::eOptimal, vk::FormatFeatureFlagBits::eDepthStencilAttachment);
}

SwapchainSupportDetails RPhysicalDevice::GetSwapchainSupportDetails() const
{
	SwapchainSupportDetails ssDetails;

	ssDetails.capabilities = getSurfaceCapabilitiesKHR(surface);
	ssDetails.formats = getSurfaceFormatsKHR(surface);
	ssDetails.presentModes = getSurfacePresentModesKHR(surface);
	return ssDetails;
}
} // namespace vl
