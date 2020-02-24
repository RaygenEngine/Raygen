#pragma once

#include "renderer/VkObjectWrapper.h"

#include <vulkan/vulkan.hpp>


class DeviceWrapper;

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


// physical device with surface support
class PhysicalDeviceWrapper : public VkObjectWrapper<vk::PhysicalDevice> {

	float m_rating{ 0.0f };

	std::vector<QueueFamily> m_graphicsFamilies;
	std::vector<QueueFamily> m_transferFamilies;
	std::vector<QueueFamily> m_computeFamilies;
	std::vector<QueueFamily> m_presentFamilies;

public:
	void Init(vk::PhysicalDevice handle, vk::SurfaceKHR surface);

	float GetDeviceRating() const { return m_rating; }

	std::vector<QueueFamily> GetGraphicsFamilies() const { return m_graphicsFamilies; }
	std::vector<QueueFamily> GetTransferFamilies() const { return m_transferFamilies; }
	std::vector<QueueFamily> GetComputeFamilies() const { return m_computeFamilies; }
	std::vector<QueueFamily> GetPresentFamilies() const { return m_presentFamilies; }

	SwapchainSupportDetails GetSwapchainSupportDetails(vk::SurfaceKHR surface);

	uint32 FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties);

	vk::Format FindSupportedFormat(
		const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, vk::FormatFeatureFlags features);

	vk::Format FindDepthFormat();
};
