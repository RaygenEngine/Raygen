#include "pch/pch.h"

#include "renderer/LogicalDevice.h"
#include "renderer/PhysicalDevice.h"
#include "renderer/Swapchain.h"
#include "asset/AssetManager.h"
#include "asset/pods/BinaryPod.h"
#include "system/Logger.h"
#include "asset/util/SpirvCompiler.h"

#include <set>

namespace {
QueueFamily GetQueueFamilyWithBestRating(const std::vector<QueueFamily>& queueFamilies)
{
	auto it = std::max_element(
		queueFamilies.begin(), queueFamilies.end(), [](QueueFamily a, QueueFamily b) { return a.rating < b.rating; });

	return { *it };
}
} // namespace

LogicalDevice::LogicalDevice(PhysicalDevice* pd, std::vector<const char*> deviceExtensions)
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

	handle = pd->handle.createDeviceUnique(deviceCreateInfo);

	// Device queues
	graphicsQueue.familyIndex = graphicsQueueFamily.index;
	graphicsQueue.handle = handle->getQueue(graphicsQueueFamily.index, 0);

	transferQueue.familyIndex = transferQueueFamily.index;
	transferQueue.handle = handle->getQueue(transferQueueFamily.index, 0);

	presentQueue.familyIndex = presentQueueFamily.index;
	presentQueue.handle = handle->getQueue(presentQueueFamily.index, 0);


	vk::CommandPoolCreateInfo graphicsPoolInfo{};
	graphicsPoolInfo.setQueueFamilyIndex(graphicsQueue.familyIndex);
	graphicsPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	graphicsCmdPool = handle->createCommandPoolUnique(graphicsPoolInfo);

	vk::CommandPoolCreateInfo transferPoolInfo{};
	transferPoolInfo.setQueueFamilyIndex(transferQueue.familyIndex);
	transferPoolInfo.setFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer);

	transferCmdPool = handle->createCommandPoolUnique(transferPoolInfo);

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandPool(transferCmdPool.get())
		.setCommandBufferCount(1u);

	transferCmdBuffer = std::move(handle->allocateCommandBuffersUnique(allocInfo)[0]);

	allocInfo.setCommandPool(graphicsCmdPool.get());

	graphicsCmdBuffer = std::move(handle->allocateCommandBuffersUnique(allocInfo)[0]);
}

vk::UniqueShaderModule LogicalDevice::CreateShaderModule(const std::string& binPath)
{
	auto& data = AssetImporterManager::ResolveOrImportFromParentUri<BinaryPod>(binPath, "/").Lock()->data;

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(data.size()).setPCode(reinterpret_cast<const uint32*>(data.data()));

	return handle->createShaderModuleUnique(createInfo);
}

vk::UniqueShaderModule LogicalDevice::CompileCreateShaderModule(const std::string& path)
{
	auto binary = ShaderCompiler::Compile(path);

	vk::ShaderModuleCreateInfo createInfo{};
	createInfo.setCodeSize(binary.size() * 4).setPCode(binary.data());

	return handle->createShaderModuleUnique(createInfo);
}

void LogicalDevice::CreateBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties,
	vk::UniqueBuffer& buffer, vk::UniqueDeviceMemory& memory)
{
	vk::BufferCreateInfo bufferInfo{};
	bufferInfo.setSize(size).setUsage(usage).setSharingMode(vk::SharingMode::eExclusive);

	buffer = handle->createBufferUnique(bufferInfo);
	vk::MemoryRequirements memRequirements = handle->getBufferMemoryRequirements(buffer.get());

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
	memory = handle->allocateMemoryUnique(allocInfo);

	handle->bindBufferMemory(buffer.get(), memory.get(), 0);
}

void LogicalDevice::CopyBuffer(vk::Buffer srcBuffer, vk::Buffer dstBuffer, vk::DeviceSize size)
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

	transferQueue.handle.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	transferQueue.handle.waitIdle();
}

void LogicalDevice::CreateImage(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
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

	image = handle->createImageUnique(imageInfo);

	vk::MemoryRequirements memRequirements = handle->getImageMemoryRequirements(image.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(pd->FindMemoryType(memRequirements.memoryTypeBits, properties));

	memory = handle->allocateMemoryUnique(allocInfo);

	handle->bindImageMemory(image.get(), memory.get(), 0);
}

void LogicalDevice::CopyBufferToImage(vk::Buffer buffer, vk::Image image, uint32 width, uint32 height)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	transferCmdBuffer->begin(beginInfo);

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

	transferCmdBuffer->copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, std::array{ region });

	transferCmdBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&transferCmdBuffer.get());

	transferQueue.handle.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	transferQueue.handle.waitIdle();
}

void LogicalDevice::TransitionImageLayout(
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
	else if (oldLayout == vk::ImageLayout::eColorAttachmentOptimal
			 && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		barrier.setDstAccessMask(vk::AccessFlagBits::eShaderRead);

		sourceStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
		destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

		// WIP: this should be a transition?
		barrier.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
	}
	else if (oldLayout == vk::ImageLayout::eShaderReadOnlyOptimal
			 && newLayout == vk::ImageLayout::eColorAttachmentOptimal) {
		barrier.setSrcAccessMask(vk::AccessFlagBits::eShaderRead);
		barrier.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);

		sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
		destinationStage = vk::PipelineStageFlagBits::eColorAttachmentOutput;

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

	graphicsCmdBuffer->begin(beginInfo);

	graphicsCmdBuffer->pipelineBarrier(
		sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });

	graphicsCmdBuffer->end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&graphicsCmdBuffer.get());

	graphicsQueue.handle.submit(1u, &submitInfo, {});
	graphicsQueue.handle.waitIdle();
}
