#pragma once
#include "rendering/wrappers/RPhysicalDevice.h"

#include <vulkan/vulkan.hpp>

namespace vl {
struct DeviceQueue : public vk::Queue {
	uint32 familyIndex;

private:
	friend struct Device_;
	void SetHandle(vk::Queue queue) { vk::Queue::operator=(queue); }
};

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
