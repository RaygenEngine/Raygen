#include "pch.h"
#include "Framebuffer.h"

#include "rendering/VulkanUtl.h"

namespace vl {

void Framebuffer::AddAttachment(UniquePtr<ImageAttachment> attachment)
{
	// TODO: add width, height tests
	CLOG_ABORT(m_hasBeenGenerated, "Attempting to generate a Framebuffer that is already generated");
	m_attachments.emplace_back(std::move(attachment));
}

void Framebuffer::Generate(vk::RenderPass renderPass)
{
	CLOG_ABORT(m_hasBeenGenerated, "Attempting to generate a Framebuffer that is already generated");

	std::vector<vk::ImageView> views;
	for (auto& att : m_attachments) {
		views.push_back(att->GetView());
	}

	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(renderPass) //
		.setAttachmentCount(static_cast<uint32>(views.size()))
		.setPAttachments(views.data())
		.setWidth(m_attachments.back()->GetExtent().width)
		.setHeight(m_attachments.back()->GetExtent().height)
		.setLayers(1);

	m_handle = Device->createFramebufferUnique(createInfo);

	m_hasBeenGenerated = true;
}

void Framebuffer::TransitionForAttachmentWrite(vk::CommandBuffer* cmdBuffer)
{
	CLOG_ABORT(!m_hasBeenGenerated, "Attempting to transition an incomplete Framebuffer");

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
