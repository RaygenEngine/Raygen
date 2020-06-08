#include "pch.h"
#include "PtRenderTarget.h"

#include "rendering/Device.h"

namespace vl {
PtRenderTarget::PtRenderTarget()
{
	// vk::AttachmentDescription colorAttachmentDesc{};
	// vk::AttachmentReference colorAttachmentRef{};

	// colorAttachmentDesc.setFormat(vk::Format::eR32G32B32A32Sfloat)
	//	.setSamples(vk::SampleCountFlagBits::e1)
	//	.setLoadOp(vk::AttachmentLoadOp::eClear)
	//	.setStoreOp(vk::AttachmentStoreOp::eStore)
	//	.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
	//	.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
	//	.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
	//	.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	// colorAttachmentRef
	//	.setAttachment(0u) //
	//	.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	// vk::SubpassDescription subpass{};
	// subpass
	//	.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
	//	.setColorAttachmentCount(1u)
	//	.setPColorAttachments(&colorAttachmentRef)
	//	.setPDepthStencilAttachment(nullptr);

	// vk::SubpassDependency dependency{};
	// dependency
	//	.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
	//	.setDstSubpass(0u)
	//	.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
	//	.setSrcAccessMask(vk::AccessFlags(0)) // 0
	//	.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
	//	.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	// vk::RenderPassCreateInfo renderPassInfo{};
	// renderPassInfo
	//	.setAttachmentCount(1u) //
	//	.setPAttachments(&colorAttachmentDesc)
	//	.setSubpassCount(1u)
	//	.setPSubpasses(&subpass)
	//	.setDependencyCount(1u)
	//	.setPDependencies(&dependency);

	// vk::RenderPass::operator=(Device->createRenderPass(renderPassInfo));
}

} // namespace vl
