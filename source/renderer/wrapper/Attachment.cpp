#include "pch.h"
#include "renderer/wrapper/Attachment.h"

#include "renderer/VulkanLayer.h"
#include "renderer/wrapper/Device.h"

Attachment::Attachment(
	uint32 width, uint32 height, vk::Format format, vk::ImageLayout initialLayout, vk::ImageUsageFlags usage)
{
	image.reset(new Image(width, height, format, vk::ImageTiling::eOptimal, initialLayout, usage,
		vk::MemoryPropertyFlagBits::eDeviceLocal));

	view = image->RequestImageView2D_0_0();
}

vk::DescriptorSet Attachment::GetDebugDescriptor()
{
	if (debugDescriptorSet) {
		return *debugDescriptorSet;
	}

	debugDescriptorSet = Layer->debugDescSetLayout.GetDescriptorSet();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(*view)
		.setSampler(*Layer->quadSampler);

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
