#pragma once

namespace vl {
struct QueueFamily {
	vk::QueueFamilyProperties props;
	uint32 index;
};

// Info about a physical device and a given surface (support details)
struct RPhysicalDevice : vk::PhysicalDevice {

	float rating{ 0.0f };

	std::vector<QueueFamily> graphicsFamilies;
	std::vector<QueueFamily> transferFamilies;
	std::vector<QueueFamily> computeFamilies;
	std::vector<QueueFamily> presentFamilies;

	RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR surface);

	vk::SurfaceKHR surface;
};
} // namespace vl
