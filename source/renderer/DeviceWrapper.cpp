#include "pch/pch.h"

#include "renderer/DeviceWrapper.h"
#include "renderer/PhysicalDeviceWrapper.h"
#include "renderer/Swapchain.h"
#include "renderer/GraphicsPipeline.h"
#include "asset/AssetManager.h"
#include "asset/pods/BinaryPod.h"
#include "system/Logger.h"
#include "asset/util/SpirvCompiler.h"

#include <set>

QueueFamily GetQueueFamilyWithBestRating(const std::vector<QueueFamily>& queueFamilies)
{
	auto it = std::max_element(
		queueFamilies.begin(), queueFamilies.end(), [](QueueFamily a, QueueFamily b) { return a.rating < b.rating; });

	return { *it };
}

void DeviceWrapper::Init(const PhysicalDeviceWrapper& physicalDevice, std::vector<const char*> deviceExtensions)
{
	m_assocPhysicalDevice = physicalDevice;

	auto graphicsQueueFamily = GetQueueFamilyWithBestRating(physicalDevice.GetGraphicsFamilies());
	auto transferQueueFamily = GetQueueFamilyWithBestRating(physicalDevice.GetTransferFamilies());
	auto presentQueueFamily = GetQueueFamilyWithBestRating(physicalDevice.GetPresentFamilies());

	// Get device's presentation queue
	std::vector<vk::DeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32> uniqueQueueFamilies
		= { graphicsQueueFamily.index, transferQueueFamily.index, presentQueueFamily.index };

	float qp1 = 1.0f;
	for (uint32 queueFamily : uniqueQueueFamilies) {
		vk::DeviceQueueCreateInfo createInfo{};
		createInfo.setQueueFamilyIndex(queueFamily).setQueueCount(1).setPQueuePriorities(&qp1);
		queueCreateInfos.push_back(createInfo);
	}

	// TODO: (get from assoc)
	vk::PhysicalDeviceFeatures deviceFeatures{};
	// WIP: check if supported by the pd..
	deviceFeatures.setSamplerAnisotropy(VK_TRUE);

	vk::DeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.setPQueueCreateInfos(queueCreateInfos.data())
		.setQueueCreateInfoCount(static_cast<uint32_t>(queueCreateInfos.size()))
		.setPEnabledFeatures(&deviceFeatures)
		.setPpEnabledExtensionNames(deviceExtensions.data())
		.setEnabledExtensionCount(static_cast<uint32>(deviceExtensions.size()))
		.setEnabledLayerCount(0);

	m_vkHandle = m_assocPhysicalDevice->createDeviceUnique(deviceCreateInfo);

	// Device queues
	m_graphicsQueue.familyIndex = graphicsQueueFamily.index;
	m_graphicsQueue.m_vkHandle = m_vkHandle->getQueue(graphicsQueueFamily.index, 0);

	m_transferQueue.familyIndex = transferQueueFamily.index;
	m_transferQueue.m_vkHandle = m_vkHandle->getQueue(transferQueueFamily.index, 0);

	m_presentQueue.familyIndex = presentQueueFamily.index;
	m_presentQueue.m_vkHandle = m_vkHandle->getQueue(presentQueueFamily.index, 0);


	vk::CommandPoolCreateInfo graphicsPoolInfo{};
	graphicsPoolInfo.setQueueFamilyIndex(m_graphicsQueue.familyIndex);
	graphicsPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	m_graphicsCommandPool = m_vkHandle->createCommandPoolUnique(graphicsPoolInfo);

	vk::CommandPoolCreateInfo transferPoolInfo{};
	transferPoolInfo.setQueueFamilyIndex(m_transferQueue.familyIndex);
	transferPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);


	m_transferCommandPool = m_vkHandle->createCommandPoolUnique(transferPoolInfo);


	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(m_transferCommandPool.get())
		.setCommandBufferCount(1u);

	m_transferCommandBuffer = std::move(m_vkHandle->allocateCommandBuffersUnique(allocInfo)[0]);


	allocInfo.setCommandPool(m_graphicsCommandPool.get());

	m_graphicsCommandBuffer = std::move(m_vkHandle->allocateCommandBuffersUnique(allocInfo)[0]);
}

vk::UniqueShaderModule DeviceWrapper::CreateShaderModule(const std::string& binPath)
{
	auto& data = AssetImporterManager::ResolveOrImportFromParentUri<BinaryPod>(binPath, "/").Lock()->data;

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(data.size()).setPCode(reinterpret_cast<const uint32*>(data.data()));

	return m_vkHandle->createShaderModuleUnique(createInfo);
}

vk::UniqueShaderModule DeviceWrapper::CompileCreateShaderModule(const std::string& path)
{
	auto binary = ShaderCompiler::Compile(path);

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(binary.size() * 4).setPCode(binary.data());

	return m_vkHandle->createShaderModuleUnique(createInfo);
}

std::unique_ptr<Texture> DeviceWrapper::CreateTexture(PodHandle<TexturePod> textPod)
{
	return std::make_unique<Texture>(*this, textPod);
}

std::unique_ptr<Swapchain> DeviceWrapper::RequestDeviceSwapchainOnSurface(vk::SurfaceKHR surface)
{
	return std::make_unique<Swapchain>(*this, surface);
}

std::unique_ptr<GraphicsPipeline> DeviceWrapper::RequestDeviceGraphicsPipeline(Swapchain* swapchain)
{
	return std::make_unique<GraphicsPipeline>(*this, swapchain);
}

std::unique_ptr<Descriptors> DeviceWrapper::RequestDeviceDescriptors(Swapchain* swapchain, GraphicsPipeline* pipeline)
{
	return std::make_unique<Descriptors>(*this, swapchain, pipeline);
}

void DeviceWrapper::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
	vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& memory)
{
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

	buffer = m_vkHandle->createBufferUnique(bufferInfo);
	vk::MemoryRequirements memRequirements = m_vkHandle->getBufferMemoryRequirements(buffer.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(m_assocPhysicalDevice.FindMemoryType(memRequirements.memoryTypeBits, properties));

	// From https://vulkan-tutorial.com/Vertex_buffers/Staging_buffer
	// It should be noted that in a real world application, you're not supposed to actually call vkAllocateMemory
	// for every individual buffer. The maximum number of simultaneous memory allocations is limited by the
	// maxMemoryAllocationCount physical device limit, which may be as low as 4096 even on high end hardware like an
	// NVIDIA GTX 1080. The right way to allocate memory for a large number of objects at the same time is to create
	// a custom allocator that splits up a single allocation among many different objects by using the offset
	// parameters that we've seen in many functions.
	memory = m_vkHandle->allocateMemoryUnique(allocInfo);

	m_vkHandle->bindBufferMemory(buffer.get(), memory.get(), 0);
}

void DeviceWrapper::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	m_transferCommandBuffer->begin(beginInfo);

	vk::BufferCopy copyRegion{};
	copyRegion.setSize(size);

	m_transferCommandBuffer->copyBuffer(srcBuffer, dstBuffer, 1, &copyRegion);

	m_transferCommandBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&m_transferCommandBuffer.get());

	m_transferQueue->submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	m_transferQueue->waitIdle();
}

void DeviceWrapper::CreateImage(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
	vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, vk::UniqueImage& image,
	vk::UniqueDeviceMemory& memory)
{
	vk::ImageCreateInfo imageInfo{};
	imageInfo.setImageType(vk::ImageType::e2D)
		.setExtent({ width, height, 1 })
		.setMipLevels(1)
		.setArrayLayers(1)
		.setFormat(format)
		.setTiling(tiling)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setUsage(usage)
		.setSamples(vk::SampleCountFlagBits::e1)
		// WIP: test
		.setSharingMode(vk::SharingMode::eExclusive);

	image = m_vkHandle->createImageUnique(imageInfo);

	vk::MemoryRequirements memRequirements = m_vkHandle->getImageMemoryRequirements(image.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(m_assocPhysicalDevice.FindMemoryType(memRequirements.memoryTypeBits, properties));

	memory = m_vkHandle->allocateMemoryUnique(allocInfo);

	m_vkHandle->bindImageMemory(image.get(), memory.get(), 0);
}

void DeviceWrapper::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32 width, uint32 height)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	m_transferCommandBuffer->begin(beginInfo);

	vk::BufferImageCopy region{};
	region.setBufferOffset(0u)
		.setBufferRowLength(0u)
		.setBufferImageHeight(0u)
		.setImageOffset({ 0, 0, 0 })
		.setImageExtent({ width, height, 1u });
	region.imageSubresource.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setMipLevel(0u)
		.setBaseArrayLayer(0u)
		.setLayerCount(1u);

	m_transferCommandBuffer->copyBufferToImage(
		buffer, image, vk::ImageLayout::eTransferDstOptimal, std::array{ region });

	m_transferCommandBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&m_transferCommandBuffer.get());

	m_transferQueue->submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	m_transferQueue->waitIdle();
}

void DeviceWrapper::TransitionImageLayout(
	vk::Image image, vk::Format format, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	vk::ImageMemoryBarrier barrier{};
	barrier.setOldLayout(oldLayout).setNewLayout(newLayout).setImage(image);

	barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0u)
		.setLevelCount(1u)
		.setBaseArrayLayer(0u)
		.setLayerCount(1u);

	if (newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth);

		// if has stencil component
		if (format == vk::Format::eD32SfloatS8Uint || format == vk::Format::eD24UnormS8Uint) {
			barrier.subresourceRange.aspectMask |= vk::ImageAspectFlagBits::eStencil;
		}
	}
	else {
		barrier.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor);
	}

	vk::PipelineStageFlags sourceStage;
	vk::PipelineStageFlags destinationStage;

	// undefined -> transfer
	if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlags{ 0u });
		barrier.setDstAccessMask(vk::AccessFlagBits::eTransferWrite);

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eTransfer;

		// WIP: is this an implicit onwership of the transfer queue?
		// it should be explicit
		barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	}
	// transfer -> graphics
	else if (oldLayout == vk::ImageLayout::eTransferDstOptimal
			 && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite);
		barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

		sourceStage = vk::PipelineStageFlagBits::eTransfer;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

		// WIP: this should be a transition?
		barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	}
	else if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eDepthStencilAttachmentOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlags{ 0u });
		barrier.setDstAccessMask(
			vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite);

		sourceStage = vk::PipelineStageFlagBits::eTopOfPipe;
		destinationStage = vk::PipelineStageFlagBits::eEarlyFragmentTests;
	}
	else {
		LOG_ABORT("Unsupported layout transition!");
	}

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	m_graphicsCommandBuffer->begin(beginInfo);

	m_graphicsCommandBuffer->pipelineBarrier(
		sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });

	m_graphicsCommandBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&m_graphicsCommandBuffer.get());

	m_graphicsQueue->submit(1u, &submitInfo, {});
	m_graphicsQueue->waitIdle();
}
