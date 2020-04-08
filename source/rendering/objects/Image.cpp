#include "pch.h"
#include "Image.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/renderer/Renderer.h"
#include "rendering/VulkanUtl.h"

namespace vl {
Image::Image(vk::ImageType imageType, vk::Extent3D extent, uint32 mipLevels, uint32 arrayLayers, vk::Format format,
	vk::ImageTiling tiling, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::SampleCountFlagBits samples,
	vk::SharingMode sharingMode, vk::MemoryPropertyFlags properties)
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
		.setSharingMode(sharingMode);

	m_handle = Device->createImageUnique(m_imageInfo);

	vk::MemoryRequirements memRequirements = Device->getImageMemoryRequirements(m_handle.get());

	vk::MemoryAllocateInfo allocInfo{};
	allocInfo.setAllocationSize(memRequirements.size);
	allocInfo.setMemoryTypeIndex(Device->pd->FindMemoryType(memRequirements.memoryTypeBits, properties));

	m_memory = Device->allocateMemoryUnique(allocInfo);

	Device->bindImageMemory(m_handle.get(), m_memory.get(), 0);
}

void Image::BlockingTransitionToLayout(vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
{
	vk::ImageMemoryBarrier barrier = CreateTransitionBarrier(oldLayout, newLayout);

	vk::PipelineStageFlags sourceStage = vl::GetPipelineStage(oldLayout);
	vk::PipelineStageFlags destinationStage = vl::GetPipelineStage(newLayout);

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
}

void Image::CopyBufferToImage(const RawBuffer& buffer)
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
		.setAspectMask(vl::GetAspectMask(m_imageInfo)) //
		.setMipLevel(0u)
		.setBaseArrayLayer(0u)
		.setLayerCount(m_imageInfo.arrayLayers);

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

vk::ImageMemoryBarrier Image::CreateTransitionBarrier(
	vk::ImageLayout oldLayout, vk::ImageLayout newLayout, uint32 baseMipLevel, uint32 baseArrayLevel)
{
	vk::ImageMemoryBarrier barrier{};
	barrier
		.setOldLayout(oldLayout) //
		.setNewLayout(newLayout)
		.setImage(m_handle.get())
		.setSrcAccessMask(vl::GetAccessMask(oldLayout))
		.setDstAccessMask(vl::GetAccessMask(newLayout))
		// CHECK: family indices
		.setSrcQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED)
		.setDstQueueFamilyIndex(VK_QUEUE_FAMILY_IGNORED);

	barrier.subresourceRange
		.setAspectMask(vl::GetAspectMask(m_imageInfo)) //
		.setBaseMipLevel(baseMipLevel)
		.setLevelCount(m_imageInfo.mipLevels)
		.setBaseArrayLayer(baseArrayLevel)
		.setLayerCount(m_imageInfo.arrayLayers);

	return barrier;
}

vk::DescriptorSet Image::GetDebugDescriptor()
{
	if (!s_imageDebugDescLayout.hasBeenGenerated) {
		s_imageDebugDescLayout.AddBinding(
			vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
		s_imageDebugDescLayout.Generate();
	}

	if (m_debugDescriptorSet) {
		return *m_debugDescriptorSet;
	}

	m_debugDescriptorSet = s_imageDebugDescLayout.GetDescriptorSet();

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
