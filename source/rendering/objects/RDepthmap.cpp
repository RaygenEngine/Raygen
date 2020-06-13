#include "pch.h"
#include "RDepthmap.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/VulkanUtl.h"

namespace vl {
RDepthmap::RDepthmap(uint32 width, uint32 height, const char* name)
{
	// attachment
	vk::Format depthFormat = Device->pd->FindDepthFormat();

	attachment = std::make_unique<RImageAttachment>(name, width, height, depthFormat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	attachment->BlockingTransitionToLayout(
		vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	// framebuffer
	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(Layouts->depthRenderPass.get()) //
		.setAttachmentCount(1u)
		.setPAttachments(&attachment->GetView())
		.setWidth(width)
		.setHeight(height)
		.setLayers(1);

	framebuffer = Device->createFramebufferUnique(createInfo);

	// description set
	descSet = Layouts->singleSamplerDescLayout.GetDescriptorSet();

	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(attachment->GetView())
		.setSampler(quadSampler);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(descSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo)
		.setPTexelBufferView(nullptr);

	Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
}

void RDepthmap::TransitionForWrite(vk::CommandBuffer* cmdBuffer)
{
	auto barrier = attachment->CreateTransitionBarrier(
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::PipelineStageFlags sourceStage = GetPipelineStage(vk::ImageLayout::eShaderReadOnlyOptimal);
	vk::PipelineStageFlags destinationStage = GetPipelineStage(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	cmdBuffer->pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
}
} // namespace vl
