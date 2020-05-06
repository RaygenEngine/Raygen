#pragma once
#include "rendering/objects/RPhysicalDevice.h"

#include <vulkan/vulkan.hpp>
#include <optional>

namespace vl {
struct DeviceQueue : public vk::Queue {
	uint32 familyIndex;

private:
	friend struct Device_;
	void SetHandle(vk::Queue queue) { vk::Queue::operator=(queue); }
};

// Info about a logical device
inline struct Device_ : public vk::Device {

	DeviceQueue graphicsQueue;
	DeviceQueue transferQueue;
	DeviceQueue presentQueue;

	RPhysicalDevice* pd;

	vk::UniqueCommandPool graphicsCmdPool;
	vk::UniqueCommandPool transferCmdPool;

	vk::CommandBuffer transferCmdBuffer;
	vk::CommandBuffer graphicsCmdBuffer;

	Device_(RPhysicalDevice* pd, std::vector<const char*> deviceExtensions);
	~Device_();
} * Device{};
} // namespace vl
