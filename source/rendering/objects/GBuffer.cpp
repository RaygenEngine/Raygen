#include "pch.h"
#include "GBuffer.h"

#include "engine/Logger.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanUtl.h"
#include "rendering/Layouts.h"

namespace vl {
GBuffer::GBuffer(vk::RenderPass renderPass, uint32 width, uint32 height)
{
	m_descSet = Layouts->gBufferDescLayout.GetDescriptorSet();

	auto initAttachment = [&](const std::string& name, vk::Format format, vk::ImageUsageFlags usage,
							  vk::ImageLayout finalLayout, bool isDepth) {
		auto att = std::make_unique<ImageAttachment>(name, width, height, format, vk::ImageLayout::eUndefined,
			usage | vk::ImageUsageFlagBits::eSampled, vk::MemoryPropertyFlagBits::eDeviceLocal, isDepth);
		att->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, finalLayout);
		return att;
	};

	for (size_t i = 0; i < 5; ++i) {
		m_framebuffer.AddAttachment(initAttachment(attachmentNames[i], colorAttachmentFormats[i],
			vk::ImageUsageFlagBits::eColorAttachment, vk::ImageLayout::eColorAttachmentOptimal, false));
	}

	vk::Format depthFormat = Device->pd->FindDepthFormat();

	m_framebuffer.AddAttachment(initAttachment(attachmentNames[GDepth], depthFormat,
		vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal, true));

	m_framebuffer.Generate(renderPass);
}

void GBuffer::TransitionForAttachmentWrite(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);

	m_framebuffer.TransitionForAttachmentWrite(cmdBuffer);
}

void GBuffer::UpdateDescriptorSet()
{
	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	auto& atts = m_framebuffer.GetAttachments();

	for (uint32 i = 0; i < GCount; ++i) {

		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(atts[i]->GetView())
			.setSampler(quadSampler);

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite
			.setDstSet(m_descSet) //
			.setDstBinding(i)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setPBufferInfo(nullptr)
			.setPImageInfo(&imageInfo)
			.setPTexelBufferView(nullptr);

		Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}
} // namespace vl
