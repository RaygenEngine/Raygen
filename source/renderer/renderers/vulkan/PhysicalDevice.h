#pragma once

#include <vulkan/vulkan.hpp>

namespace vulkan {

class Device;

struct QueueFamily {
	vk::QueueFamilyProperties props;
	uint32 familyIndex;

	float rating{ 0.0f };
};

struct SwapchainSupportDetails {
	vk::SurfaceCapabilitiesKHR capabilities;
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
};


// physical device with surface support
class PhysicalDevice : public vk::PhysicalDevice {

	float m_rating{ 0.0f };

	std::vector<QueueFamily> m_graphicsFamilies;
	std::vector<QueueFamily> m_transferFamilies;
	std::vector<QueueFamily> m_computeFamilies;
	std::vector<QueueFamily> m_presentFamilies;

public:
	PhysicalDevice(vk::PhysicalDevice handle, vk::SurfaceKHR surface);

	float GetDeviceRating() const { return m_rating; }

	QueueFamily GetBestGraphicsFamily() const;
	QueueFamily GetBestTransferFamily() const;
	QueueFamily GetBestComputeFamily() const;
	QueueFamily GetBestPresentFamily() const;

	SwapchainSupportDetails GetSwapchainSupportDetails(vk::SurfaceKHR surface);

	std::unique_ptr<Device> RequestLogicalDevice();

	uint32 FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties);

	vk::Format FindSupportedFormat(
		const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	vk::Format FindDepthFormat();
};
} // namespace vulkan
