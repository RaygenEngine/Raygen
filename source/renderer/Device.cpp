#include "pch.h"
#include "renderer/Device.h"

#include "renderer/PhysicalDevice.h"
#include "asset/AssetManager.h"
#include "asset/pods/BinaryPod.h"
#include "asset/util/SpirvCompiler.h"
#include "engine/Logger.h"
#include "renderer/Swapchain.h"

#include <set>

namespace {
QueueFamily GetQueueFamilyWithBestRating(const std::vector<QueueFamily>& queueFamilies)
{
	auto it = std::max_element(
		queueFamilies.begin(), queueFamilies.end(), [](QueueFamily a, QueueFamily b) { return a.rating < b.rating; });

	return { *it };
}
} // namespace

Device::Device(PhysicalDevice* pd, std::vector<const char*> deviceExtensions)
	: pd(pd)
{
	auto graphicsQueueFamily = GetQueueFamilyWithBestRating(pd->graphicsFamilies);
	auto transferQueueFamily = GetQueueFamilyWithBestRating(pd->transferFamilies);
	auto presentQueueFamily = GetQueueFamilyWithBestRating(pd->presentFamilies);

	// Get device's presentation queue
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32> uniqueQueueFamilies
		= { graphicsQueueFamily.index, transferQueueFamily.index, presentQueueFamily.index };

	float qp1 = 1.0f;
	for (uint32 queueFamily : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo createInfo{};
		createInfo.setQueueFamilyIndex(queueFamily)
			.setQueueCount(1) //
			.setPQueuePriorities(&qp1);
		queueCreateInfos.push_back(createInfo);
	}

	// NEXT: check if supported by the pd..
	vk::PhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.setSamplerAnisotropy(VK_TRUE);

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data())
		.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
		.setPEnabledFeatures(&deviceFeatures)
		.setPpEnabledExtensionNames(deviceExtensions.data())
		.setEnabledExtensionCount(static_cast<uint32>(deviceExtensions.size()))
		.setEnabledLayerCount(0);

	vk::Device::operator=(pd->createDevice(deviceCreateInfo));

	// Device queues
	graphicsQueue.familyIndex = graphicsQueueFamily.index;
	graphicsQueue.SetHandle(getQueue(graphicsQueueFamily.index, 0));

	transferQueue.familyIndex = transferQueueFamily.index;
	transferQueue.SetHandle(getQueue(transferQueueFamily.index, 0));

	presentQueue.familyIndex = presentQueueFamily.index;
	presentQueue.SetHandle(getQueue(presentQueueFamily.index, 0));


	vk::CommandPoolCreateInfo graphicsPoolInfo{};
	graphicsPoolInfo.setQueueFamilyIndex(graphicsQueue.familyIndex);
	graphicsPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	graphicsCmdPool = createCommandPoolUnique(graphicsPoolInfo);

	vk::CommandPoolCreateInfo transferPoolInfo{};
	transferPoolInfo.setQueueFamilyIndex(transferQueue.familyIndex);
	transferPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	transferCmdPool = createCommandPoolUnique(transferPoolInfo);

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(transferCmdPool.get())
		.setCommandBufferCount(1u);

	transferCmdBuffer = std::move(allocateCommandBuffersUnique(allocInfo)[0]);

	allocInfo.setCommandPool(graphicsCmdPool.get());

	graphicsCmdBuffer = std::move(allocateCommandBuffersUnique(allocInfo)[0]);
}

Device::~Device()
{
	// NEXT:
	// destroy();
}

vk::UniqueShaderModule Device::CreateShaderModule(const std::string& binPath)
{
	auto& data = AssetImporterManager::ResolveOrImportFromParentUri<BinaryPod>(binPath, "/").Lock()->data;

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(data.size()).setPCode(reinterpret_cast<const uint32*>(data.data()));

	return createShaderModuleUnique(createInfo);
}

vk::UniqueShaderModule Device::CompileCreateShaderModule(const std::string& path)
{
	auto binary = ShaderCompiler::Compile(path);

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(binary.size() * 4).setPCode(binary.data());

	return createShaderModuleUnique(createInfo);
}

void Device::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
	vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& memory)
{
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

	buffer = createBufferUnique(bufferInfo);
	vk::MemoryRequirements memRequirements = getBufferMemoryRequirements(buffer.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(pd->FindMemoryType(memRequirements.memoryTypeBits, properties));

	// From https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
	// It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory
	// for every individual buffer. The maximum number of simultaneous memory allocations is limited by the
	// maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an
	// NVIDIA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create
	// a custom allocator that splits up a single allocation among many different objects by using the offset
	// parameters that we've seen in many functions.
	memory = allocateMemoryUnique(allocInfo);

	bindBufferMemory(buffer.get(), memory.get(), 0);
}

void Device::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	transferCmdBuffer->begin(beginInfo);

	vk::BufferCopy copyRegion{};
	copyRegion.setSize(size);

	transferCmdBuffer->copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

	transferCmdBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&transferCmdBuffer.get());

	transferQueue.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	transferQueue.waitIdle();
}
