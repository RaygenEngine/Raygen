#include "pch.h"
#include "RDepthmap.h"

#include "rendering/VulkanUtl.h"

namespace vl {
RDepthmap::RDepthmap(vk::RenderPass renderPass, uint32 width, uint32 height, const char* name)
{
	vk::Format depthFormat = Device->pd->FindDepthFormat();

	m_depthAttachment = std::make_unique<ImageAttachment>(name, width, height, depthFormat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, true);

	m_depthAttachment->BlockingTransitionToLayout(
		vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(renderPass) //
		.setAttachmentCount(1u)
		.setPAttachments(&m_depthAttachment->GetView())
		.setWidth(width)
		.setHeight(height)
		.setLayers(1);

	m_framebuffer = Device->createFramebufferUnique(createInfo);
}

void RDepthmap::TransitionForWrite(vk::CommandBuffer* cmdBuffer)
{
	auto barrier = m_depthAttachment->CreateTransitionBarrier(
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::PipelineStageFlags sourceStage = GetPipelineStage(vk::ImageLayout::eShaderReadOnlyOptimal);
	vk::PipelineStageFlags destinationStage = GetPipelineStage(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	cmdBuffer->pipelineBarrier(sourceStage, destinationStage, vk::DependencyFlags{ 0 }, {}, {}, std::array{ barrier });
}
} // namespace vl
