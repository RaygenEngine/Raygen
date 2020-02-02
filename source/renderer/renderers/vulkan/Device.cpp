#include "pch/pch.h"

#include "renderer/renderers/vulkan/Device.h"
#include "renderer/renderers/vulkan/PhysicalDevice.h"
#include "renderer/renderers/vulkan/Swapchain.h"
#include "renderer/renderers/vulkan/GraphicsPipeline.h"
#include "asset/AssetManager.h"
#include "system/Logger.h"

#include <set>

namespace vulkan {

Device::Device(vk::Device handle, PhysicalDevice* physicalDevice)
	: vk::Device(handle)
	, m_assocPhysicalDevice(physicalDevice)
{
	auto graphicsQueueFamily = m_assocPhysicalDevice->GetBestGraphicsFamily();
	auto transferQueueFamily = m_assocPhysicalDevice->GetBestTransferFamily();
	auto presentQueueFamily = m_assocPhysicalDevice->GetBestPresentFamily();

	m_graphicsQueue = getQueue(graphicsQueueFamily.familyIndex, 0);
	m_transferQueue = getQueue(transferQueueFamily.familyIndex, 0);
	m_presentQueue = getQueue(presentQueueFamily.familyIndex, 0);

	vk::CommandPoolCreateInfo graphicsPoolInfo{};
	graphicsPoolInfo.setQueueFamilyIndex(graphicsQueueFamily.familyIndex);
	graphicsPoolInfo.setFlags(vk::CommandPoolCreateFlags(0));

	m_graphicsCommandPool = createCommandPool(graphicsPoolInfo);

	vk::CommandPoolCreateInfo transferPoolInfo{};
	transferPoolInfo.setQueueFamilyIndex(transferQueueFamily.familyIndex);
	transferPoolInfo.setFlags(vk::CommandPoolCreateFlags(0));

	m_transferCommandPool = createCommandPool(transferPoolInfo);
}

Device::~Device()
{
	destroyCommandPool(m_graphicsCommandPool);
	destroyCommandPool(m_transferCommandPool);
	destroy();
}

vk::ShaderModule Device::CreateShaderModule(const std::string& binPath)
{
	auto& data = AssetManager::GetOrCreateFromParentUri<BinaryPod>(binPath, "/").Lock()->data;

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(data.size()).setPCode(reinterpret_cast<const uint32*>(data.data()));

	return createShaderModule(createInfo);
}

std::unique_ptr<Swapchain> Device::RequestDeviceSwapchainOnSurface(vk::SurfaceKHR surface)
{
	return std::make_unique<Swapchain>(this, surface);
}

std::unique_ptr<GraphicsPipeline> Device::RequestDeviceGraphicsPipeline(Swapchain* swapchain)
{
	return std::make_unique<GraphicsPipeline>(this, swapchain);
}

std::unique_ptr<Descriptors> Device::RequestDeviceDescriptors(Swapchain* swapchain, GraphicsPipeline* pipeline)
{
	return std::make_unique<Descriptors>(this, swapchain, pipeline);
}

void Device::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
	vk::Buffer& buffer, vk::DeviceMemory& bufferMemory)

{
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

	buffer = createBuffer(bufferInfo);
	vk::MemoryRequirements memRequirements = getBufferMemoryRequirements(buffer);

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(m_assocPhysicalDevice->FindMemoryType(memRequirements.memoryTypeBits, properties));

	// From https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
	// It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory
	// for every individual buffer. The maximum number of simultaneous memory allocations is limited by the
	// maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an
	// NVIDIA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create
	// a custom allocator that splits up a single allocation among many different objects by using the offset
	// parameters that we've seen in many functions.
	bufferMemory = allocateMemory(allocInfo);

	bindBufferMemory(buffer, bufferMemory, 0);
}

void Device::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(m_transferCommandPool)
		.setCommandBufferCount(1u);

	// TODO: check if unique gets freed automatically (due to buffer-pool relationship)
	// also check index
	vk::CommandBuffer commandBuffer = allocateCommandBuffers(allocInfo)[0];

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	commandBuffer.begin(beginInfo);

	vk::BufferCopy copyRegion{};
	copyRegion.setSize(size);

	commandBuffer.copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

	commandBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&commandBuffer);

	m_transferQueue.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	m_transferQueue.waitIdle();

	freeCommandBuffers(m_transferCommandPool, 1, &commandBuffer);
}

} // namespace vulkan
