#include "pch.h"
#include "RCubemap.h"

#include "rendering/Device.h"
#include "rendering/VulkanUtl.h"

namespace vl {
RCubemap::RCubemap(uint32 dims, uint32 mipCount, vk::Format format, vk::ImageTiling tiling,
	vk::ImageLayout initalLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
	: RImage(vk::ImageType::e2D, { dims, dims, 1u }, mipCount, 6u, format, tiling, initalLayout, usage,
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

void RCubemap::CopyBuffer(const RBuffer& buffer, size_t pixelSize, uint32 mipCount)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->dmaCmdBuffer.begin(beginInfo);

	std::vector<vk::BufferImageCopy> regions;
	size_t offset{ 0llu };
	for (uint32 mip = 0u; mip < mipCount; ++mip) {

		uint32 res = m_imageInfo.extent.width / static_cast<uint32>(std::pow(2, mip));


		vk::BufferImageCopy region{};
		region
			.setBufferOffset(offset) //
			.setBufferRowLength(0u)
			.setBufferImageHeight(0u)
			.setImageOffset({ 0, 0, 0 })
			.setImageExtent({ res, res, 1u });

		region.imageSubresource
			.setAspectMask(GetAspectMask(m_imageInfo)) //
			.setMipLevel(mip)
			.setBaseArrayLayer(0u)
			.setLayerCount(6u);

		regions.push_back(region);

		offset += res * res * pixelSize * 6llu;
	}

	Device->dmaCmdBuffer.copyBufferToImage(buffer, m_handle.get(), vk::ImageLayout::eTransferDstOptimal, regions);

	Device->dmaCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setCommandBufferCount(1u) //
		.setPCommandBuffers(&Device->dmaCmdBuffer);

	Device->dmaQueue.submit(1u, &submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	Device->dmaQueue.waitIdle();
}
} // namespace vl
