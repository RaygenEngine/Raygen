#include "pch.h"
#include "PhysicalDevice.h"

#include "engine/Logger.h"

namespace vl {
RPhysicalDevice::RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR inSurface)
	: vk::PhysicalDevice(vkHandle)
	, surface(inSurface)
{
	auto queueFamilyProperties = getQueueFamilyProperties();

	auto supportsPresent = false;
	for (uint32 i = 0; const auto& qFP : queueFamilyProperties) {

		QueueFamily queueFamily{};
		queueFamily.index = i;
		queueFamily.props = qFP;
		queueFamily.supportsPresent = getSurfaceSupportKHR(i, surface);

		supportsPresent = queueFamily.supportsPresent ? true : supportsPresent;

		queueFamilies.emplace_back(queueFamily);

		i++;
	}

	auto propertiesChain = getProperties2<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesKHR>();
	genProps = propertiesChain.get<vk::PhysicalDeviceProperties2>();
	rtProps = propertiesChain.get<vk::PhysicalDeviceRayTracingPropertiesKHR>();

	auto featuresChain = getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceBufferDeviceAddressFeatures,
		vk::PhysicalDeviceRayTracingFeaturesKHR>();
	genFeats = featuresChain.get<vk::PhysicalDeviceFeatures2>();
	bufferFeats = featuresChain.get<vk::PhysicalDeviceBufferDeviceAddressFeatures>();
	rtFeats = featuresChain.get<vk::PhysicalDeviceRayTracingFeaturesKHR>();

	auto rtSupport = rtFeats.rayTracing && rtFeats.rayQuery && rtProps.shaderGroupHandleSize == 32u
					 && rtProps.maxRecursionDepth >= 1 && bufferFeats.bufferDeviceAddress;


	CLOG_ABORT(!rtSupport || !supportsPresent, "Rendering Device not supported!");
}
} // namespace vl
