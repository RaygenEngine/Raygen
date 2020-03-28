#pragma once
#include "rendering/wrapper/PhysicalDevice.h"

#include <vulkan/vulkan.hpp>
#include <optional>

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

	PhysicalDevice* pd;

	vk::UniqueCommandPool graphicsCmdPool;
	vk::UniqueCommandPool transferCmdPool;

	vk::CommandBuffer transferCmdBuffer;
	vk::CommandBuffer graphicsCmdBuffer;

	Device_(PhysicalDevice* pd, std::vector<const char*> deviceExtensions);
	~Device_();

	std::optional<vk::UniqueShaderModule> CompileCreateShaderModule(const std::string& path);
} * Device{};
