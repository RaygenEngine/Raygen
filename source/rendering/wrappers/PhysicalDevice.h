#pragma once

#include "rendering/wrappers/Queue.h"

namespace vl {
// Info about a physical device and a given surface (support details)
struct RPhysicalDevice : vk::PhysicalDevice {
	vk::PhysicalDeviceProperties2 generalProperties;
	vk::PhysicalDeviceRayTracingPropertiesKHR raytracingProperties;

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceBufferDeviceAddressFeatures,
		vk::PhysicalDeviceDescriptorIndexingFeatures, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
		vk::PhysicalDeviceRayTracingFeaturesKHR>
		featuresChain;

	std::vector<const char*> extensions;

	RQueue::Family graphicsFamily;
	RQueue::Family dmaFamily;
	RQueue::Family computeFamily;
	RQueue::Family presentFamily;

	vk::SurfaceKHR surface;

	RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR surface);
};
} // namespace vl
