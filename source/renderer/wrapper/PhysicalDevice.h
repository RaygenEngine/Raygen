#pragma once
#include <vulkan/vulkan.hpp>

struct QueueFamily {
	vk::QueueFamilyProperties props;
	uint32 index;

	float rating{ 0.0f };
};

struct SwapchainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};


// Info about a physical device and a given surface (support details)
struct PhysicalDevice : public vk::PhysicalDevice {

	float rating{ 0.0f };

	std::vector<QueueFamily> graphicsFamilies;
	std::vector<QueueFamily> transferFamilies;
	std::vector<QueueFamily> computeFamilies;
	std::vector<QueueFamily> presentFamilies;

	SwapchainSupportDetails ssDetails;

	PhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR surface);

	uint32 FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties);

	vk::Format FindSupportedFormat(
		const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	vk::Format FindDepthFormat();
};
