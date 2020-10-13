#pragma once
#include "rendering/wrappers/PhysicalDevice.h"

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
	DeviceQueue graphicsQueue;
	// async dma between host and device, PERF: should be used only for host to device transfers, device to device
	// transfers may be faster using the graphicsQueue
	DeviceQueue dmaQueue;
	// compute dedicated
	DeviceQueue computeQueue;
	// present queue
	DeviceQueue presentQueue;

	RPhysicalDevice& pd;

	vk::UniqueCommandPool graphicsCmdPool;
	vk::UniqueCommandPool dmaCmdPool;
	vk::UniqueCommandPool computeCmdPool;

	vk::CommandBuffer graphicsCmdBuffer;
	vk::CommandBuffer dmaCmdBuffer;
	vk::CommandBuffer computeCmdBuffer;

	Device_(RPhysicalDevice& pd);
	~Device_();

	[[nodiscard]] uint32 FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties) const;

	[[nodiscard]] vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates,
		vk::ImageTiling tiling, //
		vk::FormatFeatureFlags features) const;

	[[nodiscard]] vk::Format FindDepthFormat() const;

	[[nodiscard]] vk::Format FindDepthStencilFormat() const;

	[[nodiscard]] SwapchainSupportDetails GetSwapchainSupportDetails() const;
} * Device{};
} // namespace vl
