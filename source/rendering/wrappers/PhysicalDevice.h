#pragma once

namespace vl {
struct QueueFamily {
	vk::QueueFamilyProperties props;
	uint32 index{ UINT_MAX };
	bool supportsPresent{ false };
};

// Info about a physical device and a given surface (support details)
struct RPhysicalDevice : vk::PhysicalDevice {

	std::vector<QueueFamily> queueFamilies;

	vk::PhysicalDeviceProperties2 genProps;
	vk::PhysicalDeviceRayTracingPropertiesKHR rtProps;

	vk::PhysicalDeviceFeatures2 genFeats;
	vk::PhysicalDeviceBufferDeviceAddressFeatures bufferFeats;
	vk::PhysicalDeviceRayTracingFeaturesKHR rtFeats;

	vk::SurfaceKHR surface;

	RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR surface);
};
} // namespace vl
