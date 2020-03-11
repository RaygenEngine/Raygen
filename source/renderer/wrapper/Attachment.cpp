#include "pch.h"
#include "renderer/wrapper/Attachment.h"

#include "renderer/VulkanLayer.h"
#include "renderer/wrapper/Device.h"

Attachment::Attachment(uint32 width, uint32 height, vk::Format format, vk::ImageUsageFlags usage)
{
	image = std::make_unique<Image>(
		width, height, format, vk::ImageTiling::eOptimal, usage, vk::MemoryPropertyFlagBits::eDeviceLocal);

	view = image->RequestImageView2D_0_0();
}

vk::DescriptorSet Attachment::GetDebugDescriptor()
{

	if (debugDescriptorSet) {
		return *debugDescriptorSet;
	}

	debugDescriptorSet = Layer->GetDebugDescriptorSet();

	auto UpdateImageSamplerInDescriptorSet = [&](vk::ImageView& view, vk::Sampler& sampler, uint32 dstBinding) {
		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(view)
			.setSampler(sampler);

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite
			.setDstSet(*debugDescriptorSet) //
			.setDstBinding(dstBinding)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setPBufferInfo(nullptr)
			.setPImageInfo(&imageInfo)
			.setPTexelBufferView(nullptr);

		Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	};


	UpdateImageSamplerInDescriptorSet(*view, *Layer->quadSampler, 0);

	return *debugDescriptorSet;
}
