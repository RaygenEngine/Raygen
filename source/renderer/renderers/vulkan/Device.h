#pragma once

#include "renderer/renderers/vulkan/PhysicalDevice.h"
#include "renderer/renderers/vulkan/Descriptors.h"

#include <vulkan/vulkan.hpp>

namespace vulkan {

class Swapchain;
class GraphicsPipeline;

class Device : public vk::Device {

	vk::Queue m_graphicsQueue;
	vk::Queue m_transferQueue;
	vk::Queue m_presentQueue;

	PhysicalDevice* m_assocPhysicalDevice;

	vk::CommandPool m_graphicsCommandPool;
	vk::CommandPool m_transferCommandPool;

public:
	Device(vk::Device handle, PhysicalDevice* physicalDevice);
	~Device();

	vk::Queue GetGraphicsQueue() const { return m_graphicsQueue; }
	vk::Queue GetTransferQueue() const { return m_transferQueue; }
	vk::Queue GetPresentQueue() const { return m_presentQueue; }

	PhysicalDevice* GetPhysicalDevice() const { return m_assocPhysicalDevice; }

	vk::CommandPool GetGraphicsCommandPool() const { return m_graphicsCommandPool; }
	vk::CommandPool GetTransferCommandPool() const { return m_transferCommandPool; }

	vk::ShaderModule CreateShaderModule(const std::string& binPath);

	std::unique_ptr<Swapchain> RequestDeviceSwapchainOnSurface(vk::SurfaceKHR surface);
	std::unique_ptr<GraphicsPipeline> RequestDeviceGraphicsPipeline(Swapchain* swapchain);
	std::unique_ptr<Descriptors> RequestDeviceDescriptors(Swapchain* swapchain, GraphicsPipeline* pipeline);

	// TODO
	// void FreeSwapchain();
	// void FreeGraphicsPipeline();
	// void FreeDescriptors();
	// void FreeModel();


	void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
		vk::Buffer& buffer, vk::DeviceMemory& bufferMemory);
	void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);
};
} // namespace vulkan
