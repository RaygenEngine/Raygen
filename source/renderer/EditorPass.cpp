#include "pch/pch.h"

#include "renderer/EditorPass.h"
#include "renderer/VulkanLayer.h"
#include "editor/imgui/ImguiImpl.h"
#include "system/Engine.h"

void EditorPass::InitRenderPassAndFramebuffers()
{
	auto& swapchain = VulkanLayer::swapchain;
	auto& device = VulkanLayer::device;

	vk::AttachmentDescription colorAttachment{};
	colorAttachment.setFormat(swapchain->imageFormat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);


	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.setAttachment(0);
	colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentDescription depthAttachment{};
	depthAttachment.setFormat(device->pd->FindDepthFormat())
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef.setAttachment(1);
	depthAttachmentRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass.setColorAttachmentCount(1)
		.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef)
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.setAttachmentCount(static_cast<uint32>(attachments.size()))
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	m_renderPass = device->handle->createRenderPassUnique(renderPassInfo);

	m_framebuffers.clear();
	m_framebuffers.resize(swapchain->images.size());

	// framebuffers
	for (auto i = 0; i < swapchain->images.size(); ++i) {
		std::array<vk::ImageView, 2> attachments = { swapchain->imageViews[i].get(), swapchain->depthImageView.get() };
		vk::FramebufferCreateInfo createInfo{};
		createInfo.setRenderPass(m_renderPass.get())
			.setAttachmentCount(static_cast<uint32>(attachments.size()))
			.setPAttachments(attachments.data())
			.setWidth(swapchain->extent.width)
			.setHeight(swapchain->extent.height)
			.setLayers(1);

		m_framebuffers[i] = device->handle->createFramebufferUnique(createInfo);
	}
}


void EditorPass::RecordCmd(vk::CommandBuffer* cmdBuffer, vk::Framebuffer& framebuffer)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer->begin(beginInfo);
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.setRenderPass(m_renderPass.get()).setFramebuffer(framebuffer);
		// WIP: extent
		renderPassInfo.renderArea
			.setOffset({ 0, 0 }) //
			.setExtent(VulkanLayer::swapchain->extent);

		std::array<vk::ClearValue, 2> clearValues = {};
		clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[1].setDepthStencil({ 1.0f, 0 });
		renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
		renderPassInfo.setPClearValues(clearValues.data());

		// Editor Render pass binds the whole window (not just the viewport)
		cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);

		// No pipeline required here, imgui uses its own
		ImguiImpl::RenderVulkan(cmdBuffer);

		// end render pass
		cmdBuffer->endRenderPass();
	}

	// end command buffer recording
	cmdBuffer->end();
}
