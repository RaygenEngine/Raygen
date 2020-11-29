
#include "RenderPassLayout.h"

#include "rendering/Device.h"
#include "rendering/util/WriteDescriptorSets.h"

namespace vl {

RRenderPassLayout::Attachment::State StateFromLayout(vk::ImageLayout layout)
{

	using enum vk::ImageLayout;
	using enum RRenderPassLayout::Attachment::State;

	switch (layout) {
		case eShaderReadOnlyOptimal: return ShaderRead;
		case eColorAttachmentOptimal: return Color;
		case eDepthStencilAttachmentOptimal: return Depth;
		case eDepthStencilReadOnlyOptimal: return ShaderRead;
	}
	LOG_ABORT("Unfinished enum detector");
}

RRenderPassLayout::AttachmentRef RRenderPassLayout::UseExternal(
	RRenderPassLayout::AttachmentRef attachment, int32 firstUseIndex)
{
	if (auto opt = GetInternalRefOf(attachment); opt) {
		return opt.value();
	}

	auto& ref = externalAttachments.emplace_back(attachment);
	externalAttachmentsDescr.emplace_back();

	ref.thisOwner = this;
	ref.thisIndex = int32(externalAttachments.size() - 1);
	ref.firstUseIndex = firstUseIndex;
	return ref;
}

std::optional<RRenderPassLayout::AttachmentRef> RRenderPassLayout::GetInternalRefOf(AttachmentRef attachment)
{
	if (!IsExternal(attachment)) {
		return attachment;
	}

	for (auto& extAtt : externalAttachments) {
		if (extAtt.IsSame(attachment)) {
			return extAtt;
		}
	}

	return {};
}

RRenderPassLayout::RRenderPassLayout(const std::string& inName)
	: name(inName)
{
	static uint32 passLayoutIndex = 0;
	uidIndex = passLayoutIndex++;
}

RRenderPassLayout::AttachmentRef RRenderPassLayout::CreateAttachment(
	const char* name, vk::Format format, vk::ImageUsageFlags additionalUsageFlags)
{
	auto& att = internalAttachments.emplace_back();
	auto& attDescr = internalAttachmentsDescr.emplace_back();

	AttachmentRef attRef = AttachmentRef{ this, int32(internalAttachments.size() - 1) };
	attRef.thisOwner = this;
	attRef.thisIndex = int32(internalAttachments.size() - 1);

	att.format = format;
	att.isDepth = rvk::isDepthFormat(format);
	att.additionalFlags = additionalUsageFlags;
	att.name = name;

	attDescr
		.setFormat(format) //
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eUndefined);


	att.state = att.isDepth ? Attachment::State::Depth : Attachment::State::Color;

	auto& cv = clearValues.emplace_back();

	if (att.isDepth) {
		cv.setDepthStencil({ 1.f, 0 });
	}
	else {
		cv.setColor(std::array{ 0.f, 0.f, 0.f, 1.f });
	}


	return attRef;
}

void RRenderPassLayout::AddSubpass(std::vector<AttachmentRef>&& inputs, std::vector<AttachmentRef>&& outputs)
{
	int32 subpassIndex = int32(subpasses.size());

	auto& subpass = subpasses.emplace_back();

	// PERF: use proper srcSubpass (as early as possible)
	uint32 srcSubpass = subpassIndex == 0 ? VK_SUBPASS_EXTERNAL : subpassIndex - 1;

	for (auto& attParam : inputs) {
		if (IsExternal(attParam)) {
			LOG_ABORT("External attachment as input is not yet supported");
		}

		// CHECK:
		// CLOG_ABORT(attParam.IsDepth(), "Depth as input from the same renderpass not supported.");

		subpass.inputs.emplace_back(attParam);

		if (!attParam.Get().isInputAttachment) {
			internalDescLayout.AddBinding(vk::DescriptorType::eInputAttachment, vk::ShaderStageFlagBits::eFragment);
			internalInputAttachmentOrder.emplace_back(attParam);
		}
		attParam.Get().isInputAttachment = true;


		auto& dep = subpassDependencies.emplace_back();

		dep.setSrcSubpass(attParam.Get().previousOutputSubpassIndex) //
			.setDstSubpass(subpassIndex);

		if (attParam.Get().state == Attachment::State::Color) {

			dep.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
				.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		}
		else if (attParam.Get().state == Attachment::State::Depth) {

			dep.setSrcStageMask(vk::PipelineStageFlagBits::eEarlyFragmentTests)
				.setSrcAccessMask(vk::AccessFlagBits::eDepthStencilAttachmentWrite);
		}

		dep.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
			.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
			.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

		auto& attRef = subpass.vkInputs.emplace_back();
		attRef
			.setAttachment(attParam.GetAttachmentIndex()) //
			.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
	}

	for (auto& att : outputs) {
		if (IsExternal(att) && !att.IsDepth()) {
			LOG_ABORT("External non depth outputs are not yet supported");
		}

		if (IsExternal(att)) {
			auto extDepthAtt = UseExternal(att, subpassIndex);
			if (extDepthAtt.firstUseIndex != subpassIndex) {
				LOG_ABORT("External depth as output for multiple subpasses is not supported");
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
			subpass.vkDepth = std::make_unique<vk::AttachmentReference>();
			subpass.vkDepth
				->setAttachment(extDepthAtt.GetAttachmentIndex()) //
				.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

			continue;
		}

		if (att.IsDepth()) {
			CLOG_ABORT(att.Get().state != Attachment::State::Depth,
				"Found depth as output attachment but its state was not Depth Write");

			subpass.depth = att;
			subpass.vkDepth = std::make_unique<vk::AttachmentReference>();
			subpass.vkDepth
				->setAttachment(att.GetAttachmentIndex()) //
				.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
		}
		else {
			if (att.Get().state != Attachment::State::Color) {
				// CHECK: doesn't work because of subpass dependency index changes
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

		att.Get().previousOutputSubpassIndex = subpassIndex;
	}


	subpass.descr
		.setInputAttachments(subpass.vkInputs) //
		.setColorAttachments(subpass.vkColors);

	if (subpass.depth.originalOwner != nullptr) {
		subpass.descr.setPDepthStencilAttachment(subpass.vkDepth.get());
	}
}

void RRenderPassLayout::AttachmentFinalLayout(AttachmentRef att, vk::ImageLayout postRenderPassLayout)
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

	for (int32 i = 0; auto& att : internalAttachmentsDescr) {
		attachmentDescrs.emplace_back(att);
		internalAttachments[i].state = StateFromLayout(att.finalLayout);
		++i;
	}

	for (int32 i = 0; auto& att : externalAttachmentsDescr) {
		attachmentDescrs.emplace_back(att);
		externalAttachments[i].Get().state = StateFromLayout(att.finalLayout);
		++i;
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

	if (!internalDescLayout.IsEmpty()) {
		internalDescLayout.Generate();
	}
}

RenderingPassInstance RRenderPassLayout::CreatePassInstance(
	uint32 width, uint32 height, std::vector<const RImageAttachment*> externalAttachmentInstances)
{
	RenderingPassInstance rpInstance;
	rpInstance.parentPassIndex = uidIndex;
	rpInstance.parent = this;
	auto& framebuffer = rpInstance.framebuffer;

	for (auto& att : internalAttachments) {
		vk::ImageUsageFlags usageBits
			= att.isDepth ? vk::ImageUsageFlagBits::eDepthStencilAttachment | vk::ImageUsageFlagBits::eTransferDst
						  : vk::ImageUsageFlagBits::eColorAttachment;
		usageBits |= vk::ImageUsageFlagBits::eSampled;

		if (att.isInputAttachment) {
			usageBits |= vk::ImageUsageFlagBits::eInputAttachment;
		}

		vk::ImageLayout initialLayout
			= att.isDepth ? vk::ImageLayout::eDepthStencilAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal;

		usageBits |= att.additionalFlags;

		framebuffer.AddAttachment(width, height, att.format, vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
			usageBits, vk::MemoryPropertyFlagBits::eDeviceLocal, att.name, initialLayout);
	}

	CLOG_ABORT(externalAttachmentInstances.size() != externalAttachments.size(),
		"Incorrect number of attachments when generating framebuffer for render pass.");

	for (auto& att : externalAttachmentInstances) {
		framebuffer.AddExistingAttachment(*att);
	}
	framebuffer.Generate(*compatibleRenderPass);

	if (internalDescLayout.HasBeenGenerated()) {
		rpInstance.internalDescSet = internalDescLayout.AllocDescriptorSet();

		if (!internalInputAttachmentOrder.empty()) {
			std::vector<vk::ImageView> views;

			for (auto& att : internalInputAttachmentOrder) {
				CLOG_ABORT(IsExternal(att),
					"RenderPassLayout internal error, using an external attachment as input attachment.");
				views.emplace_back(framebuffer[att.GetAttachmentIndex()].view());
			}

			rvk::writeDescriptorImages(
				rpInstance.internalDescSet, 0, std::move(views), vk::DescriptorType::eInputAttachment);
		}
	}


	return rpInstance;
}

// Might be usefull to keep this around, code works but we don't ever have non eUndefined initial image layout
// void RenderingPassInstance::TransitionFramebufferForFrame(vk::CommandBuffer cmdBuffer)
//{
//	CLOG_ABORT(parent->uidIndex != parentPassIndex,
//		"Parent pass index was incorrect. Did parent RenderPassLayout of this instance move?");
//
//	for (uint32 index = 0; auto& att : framebuffer.ownedAttachments) {
//		auto finalLayout = parent->internalAttachmentsDescr[index].finalLayout;
//		auto initialLayout = parent->internalAttachmentsDescr[index].initialLayout;
//
//		if (finalLayout != initialLayout && initialLayout != vk::ImageLayout::eUndefined) {
//			// PERF: create all transition barriers and use a single one
//			att.TransitionToLayout(cmdBuffer, finalLayout, initialLayout);
//		}
//		index++;
//	}
//}

vk::RenderPass RenderingPassInstance::GetRenderPass() const
{
	return *parent->compatibleRenderPass;
}

void RenderingPassInstance::BeginRenderPassCmd(vk::CommandBuffer cmdBuffer, vk::SubpassContents subpassContents)
{
	auto& extent = framebuffer.extent;

	vk::Rect2D scissor{};
	scissor
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	vk::Viewport viewport{};
	viewport
		.setX(0) //
		.setY(0)
		.setWidth(static_cast<float>(extent.width))
		.setHeight(static_cast<float>(extent.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(*parent->compatibleRenderPass) //
		.setFramebuffer(framebuffer.handle());
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	renderPassInfo.setClearValues(parent->clearValues);

	cmdBuffer.beginRenderPass(renderPassInfo, subpassContents);

	cmdBuffer.setViewport(0, { viewport });
	cmdBuffer.setScissor(0, { scissor });
}

void RenderingPassInstance::EndRenderPassCmd(vk::CommandBuffer cmdBuffer)
{
	cmdBuffer.endRenderPass();
}

} // namespace vl
