#pragma once

#include "rendering/wrappers/Queue.h"

namespace vl {
// Info about a physical device and a given surface (support details)
struct RPhysicalDevice : vk::PhysicalDevice {

	RQueue::Family graphicsFamily;
	RQueue::Family dmaFamily;
	RQueue::Family computeFamily;
	RQueue::Family presentFamily;

	vk::PhysicalDeviceProperties2 genProps;
	vk::PhysicalDeviceRayTracingPropertiesKHR rtProps;

	vk::PhysicalDeviceFeatures2 genFeats;
	vk::PhysicalDeviceBufferDeviceAddressFeatures bufferFeats;
	vk::PhysicalDeviceRayTracingFeaturesKHR rtFeats;

	vk::SurfaceKHR surface;

	RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR surface);
};
} // namespace vl
