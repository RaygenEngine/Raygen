#pragma once
//#define VK_ENABLE_BETA_EXTENSIONS
#include <vulkan/vulkan.hpp>

namespace vl {
struct QueueFamily {
	vk::QueueFamilyProperties props;
	uint32 index;
};

struct SwapchainSupportDetails {
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
	vk::SurfaceCapabilitiesKHR capabilities;
};


// Info about a physical device and a given surface (support details)
struct RPhysicalDevice : public vk::PhysicalDevice {

	float rating{ 0.0f };

	std::vector<QueueFamily> graphicsFamilies;
	std::vector<QueueFamily> transferFamilies;
	std::vector<QueueFamily> computeFamilies;
	std::vector<QueueFamily> presentFamilies;


	RPhysicalDevice(vk::PhysicalDevice vkHandle, vk::SurfaceKHR surface);

	uint32 FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties) const;

	vk::Format FindSupportedFormat(
		const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features) const;

	vk::SurfaceKHR surface;

	vk::Format FindDepthFormat() const;

	[[nodiscard]] SwapchainSupportDetails GetSwapchainSupportDetails() const;
};
} // namespace vl
