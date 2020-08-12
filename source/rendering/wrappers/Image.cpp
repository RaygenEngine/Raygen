#include "pch.h"
#include "Image.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/VulkanUtl.h"
#include "rendering/wrappers/Buffer.h"

namespace vl {
RImage::RImage(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
	vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::SampleCountFlagBits samples,
	vk::SharingMode sharingMode, vk::ImageCreateFlags flags, vk::MemoryPropertyFlags properties,
	vk::ImageViewType viewType, const std::string& name)
	: format(format)
	, extent(extent)
	, aspectMask(GetAspectMask(usage, format))
	, samples(samples)
	, flags(flags)
	, arrayLayers(arrayLayers)
	, mipLevels(mipLevels)
	, isDepth(IsDepthFormat(format))
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

	image = Device->createImageUnique(imageInfo);

	vk::MemoryRequirements memRequirements = Device->getImageMemoryRequirements(image.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(Device->FindMemoryType(memRequirements.memoryTypeBits, properties));

	memory = Device->allocateMemoryUnique(allocInfo);

	Device->bindImageMemory(image.get(), memory.get(), 0);

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(image.get()) //
		.setViewType(viewType)
		.setFormat(format);
	viewInfo.subresourceRange
		.setAspectMask(aspectMask) //
		.setBaseMipLevel(0u)
		.setLevelCount(mipLevels)
		.setBaseArrayLayer(0u)
		.setLayerCount(arrayLayers);

	view = Device->createImageViewUnique(viewInfo);
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

	Device->dmaCmdBuffer.copyBufferToImage(buffer, image.get(), vk::ImageLayout::eTransferDstOptimal, { region });

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
		.setImageExtent(extent);

	region.imageSubresource
		.setAspectMask(aspectMask) //
		.setMipLevel(0u)
		.setBaseArrayLayer(0u)
		.setLayerCount(arrayLayers);

	Device->dmaCmdBuffer.copyImageToBuffer(image.get(), vk::ImageLayout::eTransferSrcOptimal, buffer, { region });

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

vk::ImageMemoryBarrier RImage::CreateTransitionBarrier(
	vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32 baseMipLevel, uint32 baseArrayLevel) const
{
	vk::ImageMemoryBarrier barrier{};
	barrier
		.setOldLayout(oldLayout) //
		.setNewLayout(newLayout)
		.setImage(image.get())
		.setSrcAccessMask(GetAccessMask(oldLayout))
		.setDstAccessMask(GetAccessMask(newLayout))
		// CHECK: family indices
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

	barrier.subresourceRange
		.setAspectMask(aspectMask) //
		.setBaseMipLevel(baseMipLevel)
		.setLevelCount(mipLevels)
		.setBaseArrayLayer(baseArrayLevel)
		.setLayerCount(arrayLayers);

	return barrier;
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

void RImage::TransitionToLayout(vk::CommandBuffer* cmdBuffer, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	auto barrier = CreateTransitionBarrier(oldLayout, newLayout);

	vk::PipelineStageFlags sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
	vk::PipelineStageFlags destinationStage
		= isDepth ? vk::PipelineStageFlagBits::eEarlyFragmentTests : vk::PipelineStageFlagBits::eColorAttachmentOutput;

	cmdBuffer->pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
}

void RImage::TransitionForWrite(vk::CommandBuffer* cmdBuffer) const
{
	auto toLayout
		= isDepth ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;

	auto barrier = CreateTransitionBarrier(vk::ImageLayout::eShaderReadOnlyOptimal, toLayout);

	vk::PipelineStageFlags sourceStage = vk::PipelineStageFlagBits::eFragmentShader;
	vk::PipelineStageFlags destinationStage
		= isDepth ? vk::PipelineStageFlagBits::eEarlyFragmentTests : vk::PipelineStageFlagBits::eColorAttachmentOutput;

	cmdBuffer->pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
}

void RImage::TransitionForRead(vk::CommandBuffer* cmdBuffer) const
{
	auto fromLayout
		= isDepth ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;

	auto barrier = CreateTransitionBarrier(fromLayout, vk::ImageLayout::eShaderReadOnlyOptimal);

	vk::PipelineStageFlags sourceStage
		= isDepth ? vk::PipelineStageFlagBits::eEarlyFragmentTests : vk::PipelineStageFlagBits::eColorAttachmentOutput;
	vk::PipelineStageFlags destinationStage = vk::PipelineStageFlagBits::eFragmentShader;

	cmdBuffer->pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
}

void RImage::GenerateMipmapsAndTransitionEach(vk::ImageLayout oldLayout, vk::ImageLayout finalLayout)
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

	Device->mainCmdBuffer.begin(beginInfo);

	for (uint32 layer = 0u; layer < arrayLayers; ++layer) {

		vk::ImageMemoryBarrier barrier{};
		barrier
			.setImage(image.get()) //
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

		vk::PipelineStageFlags oldStage = GetPipelineStage(oldLayout);
		vk::PipelineStageFlags intermediateStage = GetPipelineStage(intermediateLayout);
		vk::PipelineStageFlags finalStage = GetPipelineStage(finalLayout);

		for (uint32 i = 1; i < mipLevels; i++) {
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

			Device->mainCmdBuffer.blitImage(
				image.get(), intermediateLayout, image.get(), oldLayout, 1, &blit, vk::Filter::eLinear);

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
		barrier.subresourceRange.setBaseMipLevel(mipLevels - 1);

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

vk::DescriptorSet RImage::GetDebugDescriptor()
{
	if (debugDescriptorSet) {
		return *debugDescriptorSet;
	}

	debugDescriptorSet = Layouts->imageDebugDescLayout.GetDescriptorSet();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(view.get())
		.setSampler(GpuAssetManager->GetDefaultSampler());

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(*debugDescriptorSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo)
		.setPTexelBufferView(nullptr);

	Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);

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

	Device->dmaCmdBuffer.copyBufferToImage(buffer, image.get(), vk::ImageLayout::eTransferDstOptimal, regions);

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
