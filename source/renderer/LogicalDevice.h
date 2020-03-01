#pragma once

#include "renderer/PhysicalDevice.h"
#include "renderer/Texture.h"
#include "asset/pods/TexturePod.h"

#include <vulkan/vulkan.hpp>

struct DeviceQueue {
	vk::Queue handle;
	uint32 familyIndex;
};

// Info about a logical device
struct LogicalDevice {

	vk::UniqueDevice handle;

	DeviceQueue graphicsQueue;
	DeviceQueue transferQueue;
	DeviceQueue presentQueue;

	PhysicalDevice* pd;

	vk::UniqueCommandPool graphicsCmdPool;
	vk::UniqueCommandPool transferCmdPool;

	vk::UniqueCommandBuffer transferCmdBuffer;
	vk::UniqueCommandBuffer graphicsCmdBuffer;

	LogicalDevice(PhysicalDevice* pd, std::vector<const char*> deviceExtensions);

	vk::UniqueShaderModule CreateShaderModule(const std::string& binPath);
	vk::UniqueShaderModule CompileCreateShaderModule(const std::string& path);

	void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
		vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& memory);
	void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

	void CreateImage(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties, vk::UniqueImage& image, vk::UniqueDeviceMemory& memory);

	void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32 width, uint32 height);

	void TransitionImageLayout(
		vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);
};
