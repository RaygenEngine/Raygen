#pragma once

#include "renderer/PhysicalDeviceWrapper.h"
#include "renderer/Descriptors.h"
#include "renderer/Texture.h"
#include "asset/pods/TexturePod.h"

#include <vulkan/vulkan.hpp>


class Swapchain;
class GraphicsPipeline;

struct DeviceQueue : VkObjectWrapper<vk::Queue> {
	friend class DeviceWrapper;

	uint32 familyIndex;
};

class DeviceWrapper : public VkUniqueObjectWrapper<vk::UniqueDevice> {

	DeviceQueue m_graphicsQueue;
	DeviceQueue m_transferQueue;
	DeviceQueue m_presentQueue;

	PhysicalDeviceWrapper m_assocPhysicalDevice;

	vk::UniqueCommandPool m_graphicsCommandPool;
	vk::UniqueCommandPool m_transferCommandPool;

	vk::UniqueCommandBuffer m_transferCommandBuffer;
	vk::UniqueCommandBuffer m_graphicsCommandBuffer;


public:
	void Init(const PhysicalDeviceWrapper& physicalDevice, std::vector<const char*> deviceExtensions);

	PhysicalDeviceWrapper& GetPhysicalDevice() { return m_assocPhysicalDevice; }

	vk::CommandPool GetGraphicsCommandPool() const { return m_graphicsCommandPool.get(); }
	vk::CommandPool GetTransferCommandPool() const { return m_transferCommandPool.get(); }

	vk::UniqueShaderModule CreateShaderModule(const std::string& binPath);
	vk::UniqueShaderModule CompileCreateShaderModule(const std::string& path);

	std::unique_ptr<Texture> CreateTexture(PodHandle<TexturePod> textPod);

	std::unique_ptr<Swapchain> RequestDeviceSwapchainOnSurface(vk::SurfaceKHR surface);
	std::unique_ptr<GraphicsPipeline> RequestDeviceGraphicsPipeline(Swapchain* swapchain);
	std::unique_ptr<Descriptors> RequestDeviceDescriptors(Swapchain* swapchain, GraphicsPipeline* pipeline);

	void CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
		vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& memory);
	void CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size);

	void CreateImage(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
		vk::MemoryPropertyFlags properties, vk::UniqueImage& image, vk::UniqueDeviceMemory& memory);

	void CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32 width, uint32 height);

	void TransitionImageLayout(
		vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout);

	DeviceQueue GetGraphicsQueue() const { return m_graphicsQueue; }
	DeviceQueue GetTransferQueue() const { return m_transferQueue; }
	DeviceQueue GetPresentQueue() const { return m_presentQueue; }

	vk::CommandBuffer GetTransferCommandBuffer() const { return m_transferCommandBuffer.get(); }
};
