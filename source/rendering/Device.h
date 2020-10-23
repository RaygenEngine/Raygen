#pragma once
#include "rendering/wrappers/PhysicalDevice.h"
#include "rendering/wrappers/Queue.h"
#include "rendering/wrappers/CmdPool.h"

namespace vl {

struct SwapchainSupportDetails {
	std::vector<vk::SurfaceFormatKHR> formats;
	std::vector<vk::PresentModeKHR> presentModes;
	vk::SurfaceCapabilitiesKHR capabilities;
};

inline struct Device_ : public vk::Device {

	RPhysicalDevice& pd;

	Device_(RPhysicalDevice& pd);
	~Device_();

	[[nodiscard]] uint32 FindMemoryType(uint32 typeFilter, vk::MemoryPropertyFlags properties) const;

	[[nodiscard]] vk::Format FindSupportedFormat(const std::vector<vk::Format>& candidates,
		vk::ImageTiling tiling, //
		vk::FormatFeatureFlags features) const;

	[[nodiscard]] vk::Format FindDepthFormat() const;

	[[nodiscard]] vk::Format FindDepthStencilFormat() const;

	[[nodiscard]] SwapchainSupportDetails GetSwapchainSupportDetails() const;
} * Device{ nullptr };

inline struct CmdPoolManager_ {

	// graphics / transfer / compute / present
	RQueue graphicsQueue;
	// async dma between host and device, PERF: should be used only for host to device transfers, device to device
	// transfers may be faster using the graphicsQueue
	RQueue dmaQueue;
	// compute dedicated
	RQueue computeQueue;
	// present queue
	RQueue presentQueue;

	CmdPool graphicsCmdPool;
	CmdPool dmaCmdPool;
	CmdPool computeCmdPool;

	CmdPoolManager_();

} * CmdPoolManager{ nullptr };

} // namespace vl
