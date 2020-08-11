#include "pch.h"
#include "RImage.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/VulkanUtl.h"

namespace vl {
RImage::RImage(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
	vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::SampleCountFlagBits samples,
	vk::SharingMode sharingMode, vk::ImageCreateFlags flags, vk::MemoryPropertyFlags properties)
{
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
		.setSharingMode(sharingMode)
		.setFlags(flags);

	m_handle = Device->createImageUnique(m_imageInfo);

	vk::MemoryRequirements memRequirements = Device->getImageMemoryRequirements(m_handle.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(Device->FindMemoryType(memRequirements.memoryTypeBits, properties));

	m_memory = Device->allocateMemoryUnique(allocInfo);

	Device->bindImageMemory(m_handle.get(), m_memory.get(), 0);
}

void RImage::BlockingTransitionToLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	vk::ImageMemoryBarrier barrier = CreateTransitionBarrier(oldLayout, newLayout);

	vk::PipelineStageFlags sourceStage = GetPipelineStage(oldLayout);
	vk::PipelineStageFlags destinationStage = GetPipelineStage(newLayout);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->mainCmdBuffer.begin(beginInfo);

	Device->mainCmdBuffer.pipelineBarrier(
		sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });

	Device->mainCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&Device->mainCmdBuffer);

	Device->mainQueue.submit(1u, &submitInfo, {});
	Device->mainQueue.waitIdle();
}

void RImage::CopyBufferToImage(const RBuffer& buffer)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->dmaCmdBuffer.begin(beginInfo);

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

	Device->dmaCmdBuffer.copyBufferToImage(buffer, m_handle.get(), vk::ImageLayout::eTransferDstOptimal, { region });

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

void RImage::CopyImageToBuffer(const RBuffer& buffer)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->dmaCmdBuffer.begin(beginInfo);

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

	Device->dmaCmdBuffer.copyImageToBuffer(m_handle.get(), vk::ImageLayout::eTransferSrcOptimal, buffer, { region });

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

void RImage::GenerateMipmapsAndTransitionEach(vk::ImageLayout oldLayout, vk::ImageLayout finalLayout)
{
	// Check if image format supports linear blitting
	vk::FormatProperties formatProperties = Device->pd->getFormatProperties(m_imageInfo.format);

	// CHECK: from https://vulkan-tutorial.com/Generating_Mipmaps
	// There are two alternatives in this case. You could implement a function that searches common texture
	// image formats for one that does support linear blitting, or you could implement the mipmap generation in
	// software with a library like stb_image_resize. Each mip level can then be loaded into the image in the same
	// way that you loaded the original image. It should be noted that it is uncommon in practice to generate the
	// mipmap levels at runtime anyway. Usually they are pregenerated and stored in the texture file alongside the
	// base level to improve loading speed.
	CLOG_ABORT(!(formatProperties.optimalTilingFeatures & vk::FormatFeatureFlagBits::eSampledImageFilterLinear),
		"Image format does not support linear blitting!");

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->mainCmdBuffer.begin(beginInfo);

	for (uint32 layer = 0u; layer < m_imageInfo.arrayLayers; ++layer) {

		vk::ImageMemoryBarrier barrier{};
		barrier
			.setImage(m_handle.get()) //
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);
		barrier.subresourceRange
			.setAspectMask(GetAspectMask(m_imageInfo)) //
			.setBaseArrayLayer(layer)
			.setLayerCount(1u)
			.setLevelCount(1u);

		int32 mipWidth = m_imageInfo.extent.width;
		int32 mipHeight = m_imageInfo.extent.height;

		auto intermediateLayout = vk::ImageLayout::eTransferSrcOptimal;

		vk::PipelineStageFlags oldStage = GetPipelineStage(oldLayout);
		vk::PipelineStageFlags intermediateStage = GetPipelineStage(intermediateLayout);
		vk::PipelineStageFlags finalStage = GetPipelineStage(finalLayout);

		for (uint32 i = 1; i < m_imageInfo.mipLevels; i++) {
			barrier.subresourceRange.setBaseMipLevel(i - 1);

			barrier
				.setOldLayout(oldLayout) //
				.setNewLayout(intermediateLayout)
				.setSrcAccessMask(GetAccessMask(oldLayout))
				.setDstAccessMask(GetAccessMask(intermediateLayout));

			// old to intermediate
			Device->mainCmdBuffer.pipelineBarrier(
				oldStage, intermediateStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });

			vk::ImageBlit blit{};
			blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			blit.srcOffsets[1] = vk::Offset3D{ mipWidth, mipHeight, 1 };
			blit.srcSubresource
				.setAspectMask(GetAspectMask(m_imageInfo)) //
				.setMipLevel(i - 1)
				.setBaseArrayLayer(layer)
				.setLayerCount(1u);
			blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			blit.dstOffsets[1] = vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource
				.setAspectMask(GetAspectMask(m_imageInfo)) //
				.setMipLevel(i)
				.setBaseArrayLayer(layer)
				.setLayerCount(1u);

			Device->mainCmdBuffer.blitImage(
				m_handle.get(), intermediateLayout, m_handle.get(), oldLayout, 1, &blit, vk::Filter::eLinear);

			barrier
				.setOldLayout(intermediateLayout) //
				.setNewLayout(finalLayout)
				.setSrcAccessMask(GetAccessMask(intermediateLayout))
				.setDstAccessMask(GetAccessMask(finalLayout));


			// intermediate to final
			Device->mainCmdBuffer.pipelineBarrier(
				intermediateStage, finalStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });

			if (mipWidth > 1)
				mipWidth /= 2;
			if (mipHeight > 1)
				mipHeight /= 2;
		}

		// barier for final mip
		barrier.subresourceRange.setBaseMipLevel(m_imageInfo.mipLevels - 1);

		barrier
			.setOldLayout(oldLayout) //
			.setNewLayout(finalLayout)
			.setSrcAccessMask(GetAccessMask(oldLayout))
			.setDstAccessMask(GetAccessMask(finalLayout));

		Device->mainCmdBuffer.pipelineBarrier(
			oldStage, finalStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
	}

	Device->mainCmdBuffer.end();


	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&Device->mainCmdBuffer);

	Device->mainQueue.submit(1u, &submitInfo, {});
	Device->mainQueue.waitIdle();
}

void RImage::TransitionForWrite(vk::CommandBuffer* cmdBuffer)
{
	auto toLayout = IsDepthFormat(m_imageInfo.format) ? vk::ImageLayout::eDepthStencilAttachmentOptimal
													  : vk::ImageLayout::eColorAttachmentOptimal;

	auto barrier = CreateTransitionBarrier(vk::ImageLayout::eShaderReadOnlyOptimal, toLayout);

	vk::PipelineStageFlags sourceStage = GetPipelineStage(vk::ImageLayout::eShaderReadOnlyOptimal);
	vk::PipelineStageFlags destinationStage = GetPipelineStage(toLayout);

	cmdBuffer->pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
}

vk::ImageMemoryBarrier RImage::CreateTransitionBarrier(
	vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32 baseMipLevel, uint32 baseArrayLevel)
{
	vk::ImageMemoryBarrier barrier{};
	barrier
		.setOldLayout(oldLayout) //
		.setNewLayout(newLayout)
		.setImage(m_handle.get())
		.setSrcAccessMask(GetAccessMask(oldLayout))
		.setDstAccessMask(GetAccessMask(newLayout))
		// CHECK: family indices
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

	barrier.subresourceRange
		.setAspectMask(GetAspectMask(m_imageInfo)) //
		.setBaseMipLevel(baseMipLevel)
		.setLevelCount(m_imageInfo.mipLevels)
		.setBaseArrayLayer(baseArrayLevel)
		.setLayerCount(m_imageInfo.arrayLayers);

	return barrier;
}

vk::DescriptorSet RImage::GetDebugDescriptor()
{
	if (m_debugDescriptorSet) {
		return *m_debugDescriptorSet;
	}

	m_debugDescriptorSet = GpuResources->imageDebugDescLayout.GetDescriptorSet();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(m_view.get())
		.setSampler(GpuAssetManager->GetDefaultSampler());

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(*m_debugDescriptorSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo)
		.setPTexelBufferView(nullptr);

	Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);

	return *m_debugDescriptorSet;
}

} // namespace vl
