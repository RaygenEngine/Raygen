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
		auto att = std::make_unique<ImageAttachment>(name, width, height, format, vk::ImageTiling::eOptimal,
			vk::ImageLayout::eUndefined, usage | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal, isDepth);
		att->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, finalLayout);
		return att;
	};

	for (size_t i = 0; i < 5; ++i) {
		m_attachments[i] = initAttachment(attachmentNames[i], colorAttachmentFormats[i],
			vk::ImageUsageFlagBits::eColorAttachment, vk::ImageLayout::eColorAttachmentOptimal, false);
	}

	vk::Format depthFormat = Device->pd->FindDepthFormat();

	m_attachments[GDepth] = initAttachment(attachmentNames[GDepth], depthFormat,
		vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::ImageLayout::eDepthStencilAttachmentOptimal, true);

	std::array views = { m_attachments[GPosition]->GetView(), m_attachments[GNormal]->GetView(),
		m_attachments[GAlbedo]->GetView(), m_attachments[GSpecular]->GetView(), m_attachments[GEmissive]->GetView(),
		m_attachments[GDepth]->GetView() };

	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(renderPass) //
		.setAttachmentCount(static_cast<uint32>(views.size()))
		.setPAttachments(views.data())
		.setWidth(width)
		.setHeight(height)
		.setLayers(1);

	m_framebuffer = Device->createFramebufferUnique(createInfo);

	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	// CHECK: update descriptor set (is this once?)
	for (uint32 i = 0; i < GCount; ++i) {

		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(m_attachments[i]->GetView())
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

void GBuffer::TransitionForWrite(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);

	auto recordTransition = [&](auto& attachment) {
		auto target = !attachment->IsDepth() ? vk::ImageLayout::eColorAttachmentOptimal
											 : vk::ImageLayout::eDepthStencilAttachmentOptimal;

		auto barrier = attachment->CreateTransitionBarrier(vk::ImageLayout::eShaderReadOnlyOptimal, target);

		vk::PipelineStageFlags sourceStage = GetPipelineStage(vk::ImageLayout::eShaderReadOnlyOptimal);

		vk::PipelineStageFlags destinationStage = GetPipelineStage(target);

		cmdBuffer->pipelineBarrier(
			sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
	};

	for (auto& att : m_attachments) {
		recordTransition(att);
	}
}

} // namespace vl
