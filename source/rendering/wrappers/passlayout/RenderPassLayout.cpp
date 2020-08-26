#include "pch.h"

#include "RenderPassLayout.h"

#include "rendering/Device.h"

namespace vl {

RRenderPassLayout::AttachmentRef RRenderPassLayout::UseExternal(
	RRenderPassLayout::AttachmentRef attachment, int32 firstUseIndex)
{
	if (!IsExternal(attachment)) {
		return attachment;
	}

	for (auto& extAtt : externalAttachments) {
		if (extAtt.IsSame(attachment)) {
			return extAtt;
		}
	}

	auto& ref = externalAttachments.emplace_back(attachment);
	externalAttachmentsDescr.emplace_back();

	ref.thisOwner = this;
	ref.thisIndex = int32(externalAttachments.size() - 1);
	ref.firstUseIndex = firstUseIndex;
	return ref;
}


void RRenderPassLayout::AddSubpass(std::vector<AttachmentRef>&& inputs, std::vector<AttachmentRef>&& outputs)
{
	int32 subpassIndex = int32(subpasses.size());

	auto& subpass = subpasses.emplace_back();

	// PERF: use proper srcSubpass (as early as possible)
	uint32 srcSubpass = subpassIndex == 0 ? VK_SUBPASS_EXTERNAL : subpassIndex - 1;

	for (auto& attParam : inputs) {
		if (IsExternal(attParam) && !attParam.IsDepth()) {
			LOG_ABORT("External non depth inputs are not supported");
		}

		if (IsExternal(attParam) && attParam.IsDepth()) {
			auto extDepthAtt = UseExternal(attParam, subpassIndex);
			if (extDepthAtt.firstUseIndex != subpassIndex) {
				LOG_ABORT("External depth as input for multiple subpasses is not supported");
			}

			auto prevState = extDepthAtt.Get().state;

			extDepthAtt.GetDescr()
				.setFormat(extDepthAtt.GetFormat()) //
				.setSamples(vk::SampleCountFlagBits::e1)
				.setLoadOp(vk::AttachmentLoadOp::eLoad)
				.setStoreOp(vk::AttachmentStoreOp::eStore)
				.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
				.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
				.setInitialLayout(prevState == Attachment::State::ShaderRead
									  ? vk::ImageLayout::eShaderReadOnlyOptimal
									  : vk::ImageLayout::eDepthStencilAttachmentOptimal)
				.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			extDepthAtt.Get().state = Attachment::State::Depth;

			subpass.depth = extDepthAtt;
			subpass.vkDepth
				.setAttachment(extDepthAtt.GetAttachmentIndex()) //
				.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			continue;
		}

		CLOG_ABORT(attParam.IsDepth(), "Depth as input from the same renderpass not supported.");

		subpass.inputs.emplace_back(attParam);
		attParam.Get().isInputAttachment = true;

		if (attParam.Get().state != Attachment::State::ShaderRead) {
			auto& dep = subpassDependencies.emplace_back();
			dep.setSrcSubpass(srcSubpass)
				.setDstSubpass(subpassIndex)
				// WIP: CHECK: for gbuffer depth in light pass
				.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
				//
				.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
				.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
				.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
		}
		auto& attRef = subpass.vkInputs.emplace_back();
		attRef
			.setAttachment(attParam.GetAttachmentIndex()) //
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

		attParam.Get().state = Attachment::State::ShaderRead;
	}

	for (auto& att : outputs) {
		if (IsExternal(att)) {
			LOG_ABORT("External attachment as output is not supported");
		}

		if (att.Get().state != Attachment::State::Color) {
			LOG_WARN("Reading and writting at the same attachment in the same render pass");

			auto& dep = subpassDependencies.emplace_back();
			dep.setSrcSubpass(srcSubpass)
				.setDstSubpass(subpassIndex)
				.setSrcStageMask(vk::PipelineStageFlagBits::eFragmentShader)
				.setSrcAccessMask(vk::AccessFlagBits::eShaderRead)
				.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
				.setDependencyFlags(vk::DependencyFlagBits::eByRegion);
		}

		subpass.colors.emplace_back(att);

		auto& attRef = subpass.vkColors.emplace_back();
		attRef
			.setAttachment(att.GetAttachmentIndex()) //
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	}


	subpass.descr
		.setInputAttachments(subpass.vkInputs) //
		.setColorAttachments(subpass.vkColors);

	if (subpass.depth.originalOwner != nullptr) {
		subpass.descr.setPDepthStencilAttachment(&subpass.vkDepth);
	}
}

void RRenderPassLayout::TransitionAttachment(AttachmentRef att, vk::ImageLayout postRenderPassLayout)
{
	if (IsExternal(att)) {
		auto ownedAtt = UseExternal(att, -1);
		if (ownedAtt.firstUseIndex != -1) {
			ownedAtt.GetDescr().setFinalLayout(postRenderPassLayout);
		}
		else {
			LOG_ERROR("The attachment was not used in this renderpass. Set Layout will not happen.");
		}
	}
	else {
		att.GetDescr().setFinalLayout(postRenderPassLayout);
	}
}

void RRenderPassLayout::Generate()
{

	std::vector<vk::AttachmentDescription> attachmentDescrs;
	std::vector<vk::SubpassDescription> subpassDescrs;

	for (auto& att : internalAttachmentsDescr) {
		attachmentDescrs.emplace_back(att);
	}

	for (auto& att : externalAttachmentsDescr) {
		attachmentDescrs.emplace_back(att);
	}

	for (auto& subp : subpasses) {
		subpassDescrs.emplace_back(subp.descr);
	}

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachments(attachmentDescrs) //
		.setSubpasses(subpassDescrs)
		.setDependencies(subpassDependencies);

	compatibleRenderPass = Device->createRenderPassUnique(renderPassInfo);
}


} // namespace vl
