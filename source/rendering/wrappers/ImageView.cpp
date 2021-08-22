#include "ImageView.h"

#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"

namespace vl {
RImage::RImage(vk::ImageType imageType, const std::string& name, vk::Extent3D extent, vk::Format format,
	uint32 mipLevels, uint32 arrayLayers, vk::ImageLayout finalLayout, vk::MemoryPropertyFlags properties,
	vk::ImageUsageFlags usageFlags, vk::ImageCreateFlags flags, vk::ImageLayout initialLayout,
	vk::SampleCountFlagBits samples, vk::SharingMode sharingMode, vk::ImageTiling tiling)
	: format(format)
	, extent(extent)
	, aspectMask(rvk::getAspectMask(usageFlags, format))
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
		.setUsage(usageFlags)
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

	if (finalLayout != vk::ImageLayout::eUndefined) {
		BlockingTransitionToLayout(initialLayout, finalLayout);
	}

	vk::ImageViewType viewType;
	switch (imageType) {

		case vk::ImageType::e2D:

			if (flags & vk::ImageCreateFlagBits::eCubeCompatible) {
				viewType = arrayLayers == 6u ? vk::ImageViewType::eCube : vk::ImageViewType::eCubeArray;
			}
			else {
				if (flags & vk::ImageCreateFlagBits::e2DArrayCompatible) {
					viewType = vk::ImageViewType::e2DArray;
				}

				viewType = vk::ImageViewType::e2D;
			}
			break;
		case vk::ImageType::e1D:
		case vk::ImageType::e3D: LOG_ABORT("unhandled image type"); break;
	}

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(uHandle.get()) //
		.setViewType(viewType)
		.setFormat(format);

	viewInfo.subresourceRange
		.setAspectMask(aspectMask) // TODO: this aspect mask wont work for depth stencil attachment
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
	ScopedOneTimeSubmitCmdBuffer<Dma> cmdBuffer{};

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

	cmdBuffer.copyBufferToImage(buffer.handle(), uHandle.get(), vk::ImageLayout::eTransferDstOptimal, region);
}

void RImage::CopyImageToBuffer(const RBuffer& buffer)
{
	ScopedOneTimeSubmitCmdBuffer<Dma> cmdBuffer{};

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

	cmdBuffer.copyImageToBuffer(uHandle.get(), vk::ImageLayout::eTransferSrcOptimal, buffer.handle(), region);
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
	ScopedOneTimeSubmitCmdBuffer<Graphics> cmdBuffer{};

	auto barrier = CreateTransitionBarrier(oldLayout, newLayout);

	cmdBuffer.pipelineBarrier(sourceStage, destStage, vk::DependencyFlags{ 0 }, {}, {}, barrier);
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
	ScopedOneTimeSubmitCmdBuffer<Graphics> cmdBuffer{};

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
			cmdBuffer.pipelineBarrier(oldStage, intermediateStage, vk::DependencyFlags{ 0 }, {}, {}, barrier);

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

			cmdBuffer.blitImage(uHandle.get(), intermediateLayout, uHandle.get(), oldLayout, blit, vk::Filter::eLinear);

			barrier
				.setOldLayout(intermediateLayout) //
				.setNewLayout(newLayout)
				.setSrcAccessMask(rvk::accessFlagsForImageLayout(intermediateLayout))
				.setDstAccessMask(rvk::accessFlagsForImageLayout(newLayout));


			// intermediate to final
			cmdBuffer.pipelineBarrier(intermediateStage, finalStage, vk::DependencyFlags{ 0 }, {}, {}, barrier);

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

		cmdBuffer.pipelineBarrier(oldStage, finalStage, vk::DependencyFlags{ 0 }, {}, {}, barrier);
	}
}

vk::DescriptorSet RImage::GetDebugDescriptor()
{
	if (debugDescriptorSet) {
		return *debugDescriptorSet;
	}

	debugDescriptorSet = Layouts->imageDebugDescLayout.AllocDescriptorSet();

	rvk::writeDescriptorImages(*debugDescriptorSet, 0u, { uView.get() });

	return *debugDescriptorSet;
}

void RCubemap::CopyBuffer(const RBuffer& buffer, size_t pixelSize, uint32 mipCount)
{
	ScopedOneTimeSubmitCmdBuffer<Dma> cmdBuffer{};

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

	cmdBuffer.copyBufferToImage(buffer.handle(), uHandle.get(), vk::ImageLayout::eTransferDstOptimal, regions);
}

std::vector<vk::UniqueImageView> RCubemap::GetFaceViews(uint32 atMip) const
{
	std::vector<vk::UniqueImageView> faceViews;

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(uHandle.get()) //
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format);

	for (uint32 i = 0u; i < arrayLayers; ++i) {

		viewInfo.subresourceRange
			.setAspectMask(aspectMask) //
			.setBaseMipLevel(atMip)
			.setLevelCount(1u)
			.setBaseArrayLayer(i)
			.setLayerCount(1u);

		faceViews.emplace_back(Device->createImageViewUnique(viewInfo));
	}

	return faceViews;
}

std::vector<vk::UniqueImageView> RCubemap::GetMipViews() const
{
	std::vector<vk::UniqueImageView> mipViews;

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(uHandle.get()) //
		.setViewType(vk::ImageViewType::eCube)
		.setFormat(format);

	for (uint32 i = 0u; i < mipLevels; ++i) {

		viewInfo.subresourceRange
			.setAspectMask(aspectMask) //
			.setBaseMipLevel(i)
			.setLevelCount(1u)
			.setBaseArrayLayer(0u)
			.setLayerCount(VK_REMAINING_ARRAY_LAYERS);

		mipViews.emplace_back(Device->createImageViewUnique(viewInfo));
	}

	return mipViews;
}

vk::UniqueImageView RCubemap::GetMipView(uint32 atMip) const
{
	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(uHandle.get()) //
		.setViewType(vk::ImageViewType::eCube)
		.setFormat(format);

	viewInfo.subresourceRange
		.setAspectMask(aspectMask) //
		.setBaseMipLevel(atMip)
		.setLevelCount(1u)
		.setBaseArrayLayer(0u)
		.setLayerCount(6u);

	return Device->createImageViewUnique(viewInfo);
}

vk::UniqueImageView RCubemap::GetFaceArrayView(uint32 atMip) const
{
	vk::UniqueImageView faceArrayView;

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(uHandle.get()) //
		.setViewType(vk::ImageViewType::e2DArray)
		.setFormat(format);

	viewInfo.subresourceRange
		.setAspectMask(aspectMask) //
		.setBaseMipLevel(atMip)
		.setLevelCount(1u)
		.setBaseArrayLayer(0u)
		.setLayerCount(arrayLayers);

	faceArrayView = Device->createImageViewUnique(viewInfo);

	return faceArrayView;
}

std::vector<vk::UniqueImageView> RCubemapArray::GetFaceViews(uint32 atArrayIndex, uint32 atMip) const
{
	std::vector<vk::UniqueImageView> faceViews;

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(uHandle.get()) //
		.setViewType(vk::ImageViewType::e2D)
		.setFormat(format);

	for (uint32 i = 0u; i < 6u; ++i) {

		viewInfo.subresourceRange
			.setAspectMask(aspectMask) //
			.setBaseMipLevel(atMip)
			.setLevelCount(1u)
			.setBaseArrayLayer(atArrayIndex * 6u + i)
			.setLayerCount(1u);

		faceViews.emplace_back(Device->createImageViewUnique(viewInfo));
	}

	return faceViews;
}

vk::UniqueImageView RCubemapArray::GetCubemapView(uint32 atArrayIndex, uint32 atMip) const
{
	vk::ImageViewCreateInfo viewInfo{};
	viewInfo
		.setImage(uHandle.get()) //
		.setViewType(vk::ImageViewType::eCube)
		.setFormat(format);

	viewInfo.subresourceRange
		.setAspectMask(aspectMask) //
		.setBaseMipLevel(atMip)
		.setLevelCount(1u)
		.setBaseArrayLayer(atArrayIndex * 6u)
		.setLayerCount(6u);
	return Device->createImageViewUnique(viewInfo);
}
} // namespace vl
