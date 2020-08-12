#pragma once
#include "rendering/wrappers/PhysicalDevice.h"

#include <vulkan/vulkan.hpp>

namespace vl {
struct DeviceQueue : public vk::Queue {
	uint32 familyIndex;

private:
	friend struct Device_;
	void SetHandle(vk::Queue queue) { vk::Queue::operator=(queue); }
};

struct SwapchainSupportDetails {
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
	vk::SurfaceCapabilitiesKHR capabilities;
};

inline struct Device_ : public vk::Device {

	// graphics / transfer / compute / present
	DeviceQueue mainQueue;
	// async dma between host and device, PERF: should be used only for host to device transfers, device to device
	// transfers may be faster using the mainQueue
	DeviceQueue dmaQueue;
	// compute dedicated (TODO: compare)
	DeviceQueue computeQueue;
	// present queue
	DeviceQueue presentQueue;

	RPhysicalDevice& pd;

	vk::UniqueCommandPool mainCmdPool;
	vk::UniqueCommandPool dmaCmdPool;
	vk::UniqueCommandPool computeCmdPool;

	vk::CommandBuffer mainCmdBuffer;
	vk::CommandBuffer dmaCmdBuffer;
	vk::CommandBuffer computeCmdBuffer;

	Device_(RPhysicalDevice& pd, std::vector<const char*> deviceExtensions);
	~Device_();


	[[nodiscard]] vk::Format FindDepthFormat() const;

	[[nodiscard]] uint32 FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties) const;

	[[nodiscard]] vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates, vk::ImageTiling tiling, //
		vk::FormatFeatureFlags features) const;

	[[nodiscard]] SwapchainSupportDetails GetSwapchainSupportDetails() const;
} * Device{};
} // namespace vl
