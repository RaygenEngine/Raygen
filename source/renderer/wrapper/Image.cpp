#include "pch.h"
#include "renderer/wrapper/Image.h"

#include "renderer/VulkanLayer.h"
#include "renderer/wrapper/Device.h"

namespace {
vk::AccessFlags GetAccessMask(vk::ImageLayout imL)
{
	switch (imL) {
		case vk::ImageLayout::eUndefined: return vk::AccessFlags{ 0u };
		case vk::ImageLayout::eColorAttachmentOptimal: return vk::AccessFlagBits::eColorAttachmentWrite;
		case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::AccessFlagBits::eShaderRead;
		case vk::ImageLayout::eTransferDstOptimal: return vk::AccessFlagBits::eTransferWrite;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal:
			return vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
		default: LOG_ABORT("Unsupported");
	}
}

vk::PipelineStageFlags GetPipelineStage(vk::ImageLayout imL)
{
	switch (imL) {
		case vk::ImageLayout::eUndefined: return vk::PipelineStageFlagBits::eTopOfPipe;
		case vk::ImageLayout::eColorAttachmentOptimal: return vk::PipelineStageFlagBits::eColorAttachmentOutput;
		case vk::ImageLayout::eShaderReadOnlyOptimal: return vk::PipelineStageFlagBits::eFragmentShader;
		case vk::ImageLayout::eTransferDstOptimal: return vk::PipelineStageFlagBits::eTransfer;
		case vk::ImageLayout::eDepthStencilAttachmentOptimal: return vk::PipelineStageFlagBits::eEarlyFragmentTests;
		default: LOG_ABORT("Unsupported");
	}
}

// CHECK: how should we get the aspect mask of this image?
vk::ImageAspectFlags GetAspectMask(const vk::ImageCreateInfo& ici)
{
	auto aspectMask = vk::ImageAspectFlagBits::eColor;

	if (ici.usage & vk::ImageUsageFlagBits::eDepthStencilAttachment) {
		aspectMask = vk::ImageAspectFlagBits::eDepth;

		// if has stencil component
		if (ici.format == vk::Format::eD32SfloatS8Uint || ici.format == vk::Format::eD24UnormS8Uint) {
			return aspectMask | vk::ImageAspectFlagBits::eStencil;
		}
	}
	return aspectMask;
}
} // namespace

void Image::Init(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
	vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::SampleCountFlagBits samples,
	vk::SharingMode sharingMode, vk::MemoryPropertyFlags properties)
{
	auto pd = Device->pd;

	m_imageInfo //
		.setImageType(imageType)
		.setExtent(extent)
		.setMipLevels(mipLevels)
		.setArrayLayers(arrayLayers)
		.setFormat(format)
		.setTiling(tiling)
		.setInitialLayout(initialLayout)
		.setUsage(usage)
		.setSamples(samples)
		.setSharingMode(sharingMode);

	m_handle = Device->createImageUnique(m_imageInfo);

	vk::MemoryRequirements memRequirements = Device->getImageMemoryRequirements(m_handle.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(pd->FindMemoryType(memRequirements.memoryTypeBits, properties));

	m_memory = Device->allocateMemoryUnique(allocInfo);

	Device->bindImageMemory(m_handle.get(), m_memory.get(), 0);

	m_currentLayout = initialLayout;
}

Image::Image(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
	vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::SampleCountFlagBits samples,
	vk::SharingMode sharingMode, vk::MemoryPropertyFlags properties)
{
	Init(imageType, extent, mipLevels, arrayLayers, format, tiling, initialLayout, usage, samples, sharingMode,
		properties);
}

Image::Image(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage,
	vk::MemoryPropertyFlags properties)
{
	Init(vk::ImageType::e2D, { width, height, 1u }, 1u, 1u, format, tiling, vk::ImageLayout::eUndefined, usage,
		vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, properties);
}


Image::Image(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling, vk::ImageLayout initalLayout,
	vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
{
	Init(vk::ImageType::e2D, { width, height, 1u }, 1u, 1u, format, tiling, initalLayout, usage,
		vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, properties);
}

vk::UniqueImageView Image::RequestImageView2D_0_0()
{
	auto testComp = [&]() {
		return m_imageInfo.extent.width >= 1 && m_imageInfo.extent.height >= 1 && m_imageInfo.extent.depth == 1
			   && m_imageInfo.arrayLayers >= 1 && m_imageInfo.samples == vk::SampleCountFlagBits::e1;
	};

	CLOG_ABORT(!testComp(), "Could not create view");

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(m_handle.get()) //
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(m_imageInfo.format);
	viewInfo.subresourceRange
		.setAspectMask(GetAspectMask(m_imageInfo)) //
		.setBaseMipLevel(0u)
		.setLevelCount(1u)
		.setBaseArrayLayer(0u)
		.setLayerCount(1u);

	return Device->createImageViewUnique(viewInfo);
}

// PERF: benchmark
// TEST:
void Image::TransitionToLayout(vk::ImageLayout newLayout)
{
	if (m_currentLayout == newLayout) {
		LOG_WARN("Image layout transition, new layout is the same with the current layout of the image");
		return;
	}

	vk::ImageMemoryBarrier barrier{};
	barrier //
		.setOldLayout(m_currentLayout)
		.setNewLayout(newLayout)
		.setImage(m_handle.get())
		.setSrcAccessMask(GetAccessMask(m_currentLayout))
		.setDstAccessMask(GetAccessMask(newLayout))
		// CHECK: family indices
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

	barrier.subresourceRange
		.setAspectMask(GetAspectMask(m_imageInfo)) //
		.setBaseMipLevel(0u)
		.setLevelCount(m_imageInfo.mipLevels)
		.setBaseArrayLayer(0u)
		.setLayerCount(m_imageInfo.arrayLayers);


	vk::PipelineStageFlags sourceStage = GetPipelineStage(m_currentLayout);
	vk::PipelineStageFlags destinationStage = GetPipelineStage(newLayout);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->graphicsCmdBuffer.begin(beginInfo);

	Device->graphicsCmdBuffer.pipelineBarrier(
		sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });

	Device->graphicsCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&Device->graphicsCmdBuffer);

	Device->graphicsQueue.submit(1u, &submitInfo, {});
	Device->graphicsQueue.waitIdle();

	// change layout
	m_currentLayout = newLayout;
}

void Image::CopyBufferToImage(const Buffer& buffer)
{
	// transition layout to transfer optimal if not already
	if (m_currentLayout != vk::ImageLayout::eTransferDstOptimal) {
		TransitionToLayout(vk::ImageLayout::eTransferDstOptimal);
	}

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->transferCmdBuffer.begin(beginInfo);

	vk::BufferImageCopy region{};
	region
		.setBufferOffset(0u) //
		.setBufferRowLength(0u)
		.setBufferImageHeight(0u)
		.setImageOffset({ 0, 0, 0 })
		.setImageExtent(m_imageInfo.extent);

	region.imageSubresource
		.setAspectMask(GetAspectMask(m_imageInfo)) //
		.setMipLevel(0u)
		.setBaseArrayLayer(0u)
		.setLayerCount(m_imageInfo.arrayLayers);

	// m_currentLayout is ensured to be vk::ImageLayout::eTransferDstOptimal here
	Device->transferCmdBuffer.copyBufferToImage(buffer, m_handle.get(), m_currentLayout, std::array{ region });

	Device->transferCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setCommandBufferCount(1u) //
		.setPCommandBuffers(&Device->transferCmdBuffer);

	Device->transferQueue.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	Device->transferQueue.waitIdle();
}

vk::ImageMemoryBarrier Image::CreateTransitionBarrier(vk::ImageLayout currentLayout, vk::ImageLayout newLayout)
{
	m_currentLayout = currentLayout;

	vk::ImageMemoryBarrier barrier{};
	barrier //
		.setOldLayout(m_currentLayout)
		.setNewLayout(newLayout)
		.setImage(m_handle.get())
		.setSrcAccessMask(GetAccessMask(m_currentLayout))
		.setDstAccessMask(GetAccessMask(newLayout))
		// CHECK: family indices
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

	barrier.subresourceRange
		.setAspectMask(GetAspectMask(m_imageInfo)) //
		.setBaseMipLevel(0u)
		.setLevelCount(m_imageInfo.mipLevels)
		.setBaseArrayLayer(0u)
		.setLayerCount(m_imageInfo.arrayLayers);

	return barrier;
}
