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

	// TODO: Requirements
	//if (presentFamilies.empty() || !rayTracingFeatures.rayTracing || !rayTracingFeatures.rayQuery
	//	|| rayTracingProperties.shaderGroupHandleSize != 32u || rayTracingProperties.maxRecursionDepth < 1) {
	//	rating = 0.f;
	//}
}
} // namespace vl
