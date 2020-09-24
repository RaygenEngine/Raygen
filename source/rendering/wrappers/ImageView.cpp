#include "pch.h"
#include "ImageView.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/wrappers/Buffer.h"

namespace vl {
RImage::RImage(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
	vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::SampleCountFlagBits samples,
	vk::SharingMode sharingMode, vk::ImageCreateFlags flags, vk::MemoryPropertyFlags properties,
	vk::ImageViewType viewType, const std::string& name)
	: format(format)
	, extent(extent)
	, aspectMask(rvk::getAspectMask(usage, format))
	, samples(samples)
	, flags(flags)
	, arrayLayers(arrayLayers)
	, mipLevels(mipLevels)
	, isDepth(rvk::isDepthFormat(format))
	, name(name)
{
	vk::ImageCreateInfo imageInfo{};
	imageInfo
		.setImageType(imageType) //
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

	uHandle = Device->createImageUnique(imageInfo);

	auto memRequirements = Device->getImageMemoryRequirements(uHandle.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(Device->FindMemoryType(memRequirements.memoryTypeBits, properties));

	uMemory = Device->allocateMemoryUnique(allocInfo);

	Device->bindImageMemory(uHandle.get(), uMemory.get(), 0);

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(uHandle.get()) //
		.setViewType(viewType)
		.setFormat(format);

	viewInfo.subresourceRange
		.setAspectMask(aspectMask) //
		.setBaseMipLevel(0u)
		.setLevelCount(mipLevels)
		.setBaseArrayLayer(0u)
		.setLayerCount(arrayLayers);

	uView = Device->createImageViewUnique(viewInfo);

	DEBUG_NAME(uHandle, name);
	DEBUG_NAME(uView, name + ".view");
	DEBUG_NAME(uMemory, name + ".mem");
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
		.setImageExtent(extent);

	region.imageSubresource
		.setAspectMask(aspectMask) //
		.setMipLevel(0u)
		.setBaseArrayLayer(0u)
		.setLayerCount(arrayLayers);

	Device->dmaCmdBuffer.copyBufferToImage(
		buffer.handle(), uHandle.get(), vk::ImageLayout::eTransferDstOptimal, region);

	Device->dmaCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBuffers(Device->dmaCmdBuffer);

	Device->dmaQueue.submit(submitInfo, {});
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
		.setImageExtent(extent);

	region.imageSubresource
		.setAspectMask(aspectMask) //
		.setMipLevel(0u)
		.setBaseArrayLayer(0u)
		.setLayerCount(arrayLayers);

	Device->dmaCmdBuffer.copyImageToBuffer(
		uHandle.get(), vk::ImageLayout::eTransferSrcOptimal, buffer.handle(), region);

	Device->dmaCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBuffers(Device->dmaCmdBuffer);

	Device->dmaQueue.submit(submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	Device->dmaQueue.waitIdle();
}

vk::ImageMemoryBarrier RImage::CreateTransitionBarrier(
	vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32 baseMipLevel, uint32 baseArrayLevel) const
{
	vk::ImageMemoryBarrier barrier{};
	barrier
		.setOldLayout(oldLayout) //
		.setNewLayout(newLayout)
		.setImage(uHandle.get())
		.setSrcAccessMask(rvk::accessFlagsForImageLayout(oldLayout))
		.setDstAccessMask(rvk::accessFlagsForImageLayout(newLayout))
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

	barrier.subresourceRange
		.setAspectMask(aspectMask) //
		.setBaseMipLevel(baseMipLevel)
		.setLevelCount(VK_REMAINING_MIP_LEVELS)
		.setBaseArrayLayer(baseArrayLevel)
		.setLayerCount(VK_REMAINING_ARRAY_LAYERS);

	return barrier;
}

void RImage::BlockingTransitionToLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	BlockingTransitionToLayout(
		oldLayout, newLayout, rvk::pipelineStageForLayout(oldLayout), rvk::pipelineStageForLayout(newLayout));
}

void RImage::BlockingTransitionToLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	vk::PipelineStageFlags sourceStage, vk::PipelineStageFlags destStage)
{
	auto barrier = CreateTransitionBarrier(oldLayout, newLayout);

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->graphicsCmdBuffer.begin(beginInfo);

	Device->graphicsCmdBuffer.pipelineBarrier(sourceStage, destStage, vk::DependencyFlags{ 0 }, {}, {}, barrier);

	Device->graphicsCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBuffers(Device->graphicsCmdBuffer);

	Device->graphicsQueue.submit(submitInfo, {});

	Device->graphicsQueue.waitIdle();
}

void RImage::TransitionToLayout(vk::CommandBuffer cmdBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout) const
{
	TransitionToLayout(cmdBuffer, oldLayout, newLayout, rvk::pipelineStageForLayout(oldLayout),
		rvk::pipelineStageForLayout(newLayout));
}

void RImage::TransitionToLayout(vk::CommandBuffer cmdBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
	vk::PipelineStageFlags sourceStage, vk::PipelineStageFlags destStage) const
{
	auto barrier = CreateTransitionBarrier(oldLayout, newLayout);

	cmdBuffer.pipelineBarrier(sourceStage, destStage, vk::DependencyFlags{ 0 }, {}, {}, barrier);
}

void RImage::GenerateMipmapsAndTransitionEach(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	// Check if image format supports linear blitting
	vk::FormatProperties formatProperties = Device->pd.getFormatProperties(format);

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

	Device->graphicsCmdBuffer.begin(beginInfo);

	for (uint32 layer = 0u; layer < arrayLayers; ++layer) {

		vk::ImageMemoryBarrier barrier{};
		barrier
			.setImage(uHandle.get()) //
			.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
			.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

		barrier.subresourceRange
			.setAspectMask(aspectMask) //
			.setBaseArrayLayer(layer)
			.setLayerCount(1u)
			.setLevelCount(1u);

		int32 mipWidth = extent.width;
		int32 mipHeight = extent.height;

		auto intermediateLayout = vk::ImageLayout::eTransferSrcOptimal;

		auto oldStage = rvk::pipelineStageForLayout(oldLayout);
		auto intermediateStage = rvk::pipelineStageForLayout(intermediateLayout);
		auto finalStage = rvk::pipelineStageForLayout(newLayout);

		for (uint32 i = 1; i < mipLevels; i++) {
			barrier.subresourceRange.setBaseMipLevel(i - 1);

			barrier
				.setOldLayout(oldLayout) //
				.setNewLayout(intermediateLayout)
				.setSrcAccessMask(rvk::accessFlagsForImageLayout(oldLayout))
				.setDstAccessMask(rvk::accessFlagsForImageLayout(intermediateLayout));

			// old to intermediate
			Device->graphicsCmdBuffer.pipelineBarrier(
				oldStage, intermediateStage, vk::DependencyFlags{ 0 }, {}, {}, barrier);

			vk::ImageBlit blit{};
			blit.srcOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			blit.srcOffsets[1] = vk::Offset3D{ mipWidth, mipHeight, 1 };
			blit.srcSubresource
				.setAspectMask(aspectMask) //
				.setMipLevel(i - 1)
				.setBaseArrayLayer(layer)
				.setLayerCount(1u);
			blit.dstOffsets[0] = vk::Offset3D{ 0, 0, 0 };
			blit.dstOffsets[1] = vk::Offset3D{ mipWidth > 1 ? mipWidth / 2 : 1, mipHeight > 1 ? mipHeight / 2 : 1, 1 };
			blit.dstSubresource
				.setAspectMask(aspectMask) //
				.setMipLevel(i)
				.setBaseArrayLayer(layer)
				.setLayerCount(1u);

			Device->graphicsCmdBuffer.blitImage(
				uHandle.get(), intermediateLayout, uHandle.get(), oldLayout, blit, vk::Filter::eLinear);

			barrier
				.setOldLayout(intermediateLayout) //
				.setNewLayout(newLayout)
				.setSrcAccessMask(rvk::accessFlagsForImageLayout(intermediateLayout))
				.setDstAccessMask(rvk::accessFlagsForImageLayout(newLayout));


			// intermediate to final
			Device->graphicsCmdBuffer.pipelineBarrier(
				intermediateStage, finalStage, vk::DependencyFlags{ 0 }, {}, {}, barrier);

			if (mipWidth > 1)
				mipWidth /= 2;
			if (mipHeight > 1)
				mipHeight /= 2;
		}

		// barier for final mip
		barrier.subresourceRange.setBaseMipLevel(mipLevels - 1);

		barrier
			.setOldLayout(oldLayout) //
			.setNewLayout(newLayout)
			.setSrcAccessMask(rvk::accessFlagsForImageLayout(oldLayout))
			.setDstAccessMask(rvk::accessFlagsForImageLayout(newLayout));

		Device->graphicsCmdBuffer.pipelineBarrier(oldStage, finalStage, vk::DependencyFlags{ 0 }, {}, {}, barrier);
	}

	Device->graphicsCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBuffers(Device->graphicsCmdBuffer);

	Device->graphicsQueue.submit(submitInfo, {});

	Device->graphicsQueue.waitIdle();
}

vk::DescriptorSet RImage::GetDebugDescriptor()
{
	if (debugDescriptorSet) {
		return *debugDescriptorSet;
	}

	debugDescriptorSet = Layouts->imageDebugDescLayout.AllocDescriptorSet();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(uView.get())
		.setSampler(GpuAssetManager->GetDefaultSampler());

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(*debugDescriptorSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setImageInfo(imageInfo);

	Device->updateDescriptorSets(descriptorWrite, nullptr);

	return *debugDescriptorSet;
}

void RCubemap::CopyBuffer(const RBuffer& buffer, size_t pixelSize, uint32 mipCount)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->dmaCmdBuffer.begin(beginInfo);

	std::vector<vk::BufferImageCopy> regions;
	size_t offset{ 0llu };
	for (uint32 mip = 0u; mip < mipCount; ++mip) {

		uint32 res = extent.width / static_cast<uint32>(std::pow(2, mip));


		vk::BufferImageCopy region{};
		region
			.setBufferOffset(offset) //
			.setBufferRowLength(0u)
			.setBufferImageHeight(0u)
			.setImageOffset({ 0, 0, 0 })
			.setImageExtent({ res, res, 1u });

		region.imageSubresource
			.setAspectMask(aspectMask) //
			.setMipLevel(mip)
			.setBaseArrayLayer(0u)
			.setLayerCount(6u);

		regions.push_back(region);

		offset += res * res * pixelSize * 6llu;
	}

	Device->dmaCmdBuffer.copyBufferToImage(
		buffer.handle(), uHandle.get(), vk::ImageLayout::eTransferDstOptimal, regions);

	Device->dmaCmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBuffers(Device->dmaCmdBuffer);

	Device->dmaQueue.submit(submitInfo, {});
	// PERF:
	// A fence would allow you to schedule multiple transfers simultaneously and wait for all of them complete,
	// instead of executing one at a time. That may give the driver more opportunities to optimize.
	Device->dmaQueue.waitIdle();
}


vl::RImage2D vl::RImage2D::Create(const std::string& name, vk::Extent2D extent, vk::Format format,
	vk::ImageLayout finalLayout, vk::ImageUsageFlags usageFlags, uint32 mipLevels, vk::MemoryPropertyFlags memoryFlags,
	vk::ImageTiling tiling)
{
	auto img = RImage2D(extent.width, extent.height, mipLevels, format, tiling, vk::ImageLayout::eUndefined, usageFlags,
		memoryFlags, name);

	if (finalLayout != vk::ImageLayout::eUndefined) {
		img.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, finalLayout, vk::PipelineStageFlagBits::eTopOfPipe,
			vk::PipelineStageFlagBits::eFragmentShader | vk::PipelineStageFlagBits::eRayTracingShaderKHR);
	}
	return img;
}

} // namespace vl
