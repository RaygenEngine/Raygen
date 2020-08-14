#pragma once

namespace vl {
struct QueueFamily {
	vk::QueueFamilyProperties props;
	uint32 index{ UINT_MAX };
	bool supportsPresent{ false };
};

// Info about a physical device and a given surface (support details)
struct RPhysicalDevice : vk::PhysicalDevice {

#define PHYSDEV_PROPS vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesKHR

	std::vector<QueueFamily> queueFamilies;

	RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR surface);

	vk::StructureChain<vk::PhysicalDeviceProperties2, vk::PhysicalDeviceRayTracingPropertiesKHR> propertiesChain;

	vk::PhysicalDeviceProperties2 genProps;
	vk::PhysicalDeviceRayTracingPropertiesKHR rtProps;

	vk::PhysicalDeviceFeatures2 genFeats;
	vk::PhysicalDeviceBufferDeviceAddressFeatures bufferFeats;
	vk::PhysicalDeviceRayTracingFeaturesKHR rtFeats;

	vk::SurfaceKHR surface;
};
} // namespace vl
