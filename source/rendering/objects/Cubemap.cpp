#include "pch.h"
#include "Cubemap.h"

#include "rendering/Device.h"
#include "rendering/VulkanUtl.h"

namespace vl {
Cubemap::Cubemap(uint32 dims, vk::Format format, vk::ImageTiling tiling, vk::ImageLayout initalLayout,
	vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
	: Image(vk::ImageType::e2D, { dims, dims, 1u }, 1u, 6u, format, tiling, initalLayout, usage,
		vk::SampleCountFlagBits::e1, vk::SharingMode::eExclusive, vk::ImageCreateFlagBits::eCubeCompatible, properties)
{
	auto testComp = m_imageInfo.extent.width >= 1 && m_imageInfo.extent.height == m_imageInfo.extent.width
					&& m_imageInfo.extent.depth == 1 && m_imageInfo.arrayLayers == 6u
					&& m_imageInfo.samples == vk::SampleCountFlagBits::e1
					&& m_imageInfo.flags & vk::ImageCreateFlagBits::eCubeCompatible;

	CLOG_ABORT(!testComp, "Could not create view");

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(m_handle.get()) //
		.setViewType(vk::ImageViewType::eCube)
		.setFormat(m_imageInfo.format);
	viewInfo.subresourceRange
		.setAspectMask(GetAspectMask(m_imageInfo)) //
		.setBaseMipLevel(0u)
		.setLevelCount(m_imageInfo.mipLevels)
		.setBaseArrayLayer(0u)
		.setLayerCount(m_imageInfo.arrayLayers);

	m_view = Device->createImageViewUnique(viewInfo);
}

// TODO: pass pointer and size and absract the buffer
void Cubemap::CopyBufferToFace(const RawBuffer& buffer, uint32 face)

{
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
		// copy to this face
		.setBaseArrayLayer(face)
		.setLayerCount(1u);

	Device->transferCmdBuffer.copyBufferToImage(
		buffer, m_handle.get(), vk::ImageLayout::eTransferDstOptimal, { region });

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
} // namespace vl
