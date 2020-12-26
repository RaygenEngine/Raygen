#pragma once

#include "rendering/wrappers/Queue.h"

namespace vl {
// Info about a physical device and a given surface (support details)
struct RPhysicalDevice : vk::PhysicalDevice {
	vk::PhysicalDeviceProperties2 generalProperties;
	vk::PhysicalDeviceRayTracingPipelinePropertiesKHR raytracingProperties;

	vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceBufferDeviceAddressFeatures,
		vk::PhysicalDeviceDescriptorIndexingFeatures, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
		vk::PhysicalDeviceAccelerationStructureFeaturesKHR, vk::PhysicalDeviceRayTracingPipelineFeaturesKHR,
		vk::PhysicalDeviceRayQueryFeaturesKHR>
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
