#pragma once
#include "renderer/PhysicalDevice.h"
#include "asset/pods/TexturePod.h"
#include "renderer/Texture.h"

#include <vulkan/vulkan.hpp>

struct DeviceQueue : public vk::Queue {
	uint32 familyIndex;

private:
	friend struct Device;
	void SetHandle(vk::Queue queue) { vk::Queue::operator=(queue); }
};

// Info about a logical device
struct Device : public vk::Device {

	DeviceQueue graphicsQueue;
	DeviceQueue transferQueue;
	DeviceQueue presentQueue;

	PhysicalDevice* pd;

	vk::UniqueCommandPool graphicsCmdPool;
	vk::UniqueCommandPool transferCmdPool;

	vk::UniqueCommandBuffer transferCmdBuffer;
	vk::UniqueCommandBuffer graphicsCmdBuffer;

	Device(PhysicalDevice* pd, std::vector<const char*> deviceExtensions);
	~Device();

	vk::UniqueShaderModule CreateShaderModule(const std::string& binPath);
	vk::UniqueShaderModule CompileCreateShaderModule(const std::string& path);

	void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
		vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& memory);
	void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
};
