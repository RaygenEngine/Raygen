#pragma once

#include "rendering/wrappers/Framebuffer.h"

namespace vl {

enum class FormatUtl
{
	Depth,
	RGBA32_sFloat,
	RGBA32_uNorm,
};


struct RRenderPassLayout {

	struct Attachment {
		vk::AttachmentDescription descr;
		enum class State
		{
			ShaderRead,
			Depth,
			Color,
		} state; // current subpass state.

		State finalState;

		vk::ImageUsageFlags additionalFlags{};

		bool isDepth{ false };
		bool isInputAttachment{ false };
	};

	struct AttachmentRef {
		RRenderPassLayout* parent{ nullptr };
		int32 index{ -1 };
		Attachment& Get() { return parent->internalAttachments[index]; }
		void SetFinalLayout(vk::ImageLayout layout) { Get().descr.setFinalLayout(layout); }
	};


	struct Subpass {
		AttachmentRef depth;
		std::vector<AttachmentRef> inputs;
		std::vector<AttachmentRef> colors;
		vk::SubpassDescription descr;

	private:
		friend struct RRenderPassLayout;

		std::vector<vk::AttachmentReference> vkInputs;
		std::vector<vk::AttachmentReference> vkColors;
		vk::AttachmentReference vkDepth;
	};

	//
	//

	vk::UniqueRenderPass compatibleRenderPass;

	std::vector<vk::SubpassDependency> subpassDependencies;

	std::vector<Attachment> internalAttachments;
	std::vector<Subpass> subpasses;


	AttachmentRef AddExternalAttachment() {}


	AttachmentRef AddInternalAttachment(vk::Format format, vk::ImageUsageFlags additionalUsageFlags = {})
	{
		auto& att = internalAttachments.emplace_back();

		att.isDepth = rvk::isDepthFormat(format);
		att.descr
			.setFormat(format) //
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			// PERF: Could be usefull to have undefined here to avoid an expensive transition at the
			// end of the frame
			.setInitialLayout(
				att.isDepth ? vk::ImageLayout::eDepthAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eUndefined);

		att.state = att.isDepth ? Attachment::State::Depth : Attachment::State::Color;
		att.additionalFlags = additionalUsageFlags;

		return { this, int32(internalAttachments.size() - 1) };
	}


	void AddSubpass(std::vector<AttachmentRef>&& inputs, std::vector<AttachmentRef>&& outputs)
	{
		CLOG_ABORT(subpasses.empty() && !inputs.empty(), "Found inputs at first subpass");
		int32 subpassIndex = int32(subpasses.size());

		auto& subpass = subpasses.emplace_back();

		uint32 srcSubpass = subpassIndex == 0 ? VK_SUBPASS_EXTERNAL : subpassIndex - 1;

		for (auto& att : outputs) {
			if (att.Get().isDepth) {
				if (att.Get().state != Attachment::State::Depth) {
					LOG_ERROR(
						"Attempting to read from an unwritten depth attachment. Otherwise the attachment was not in "
						"depth state.");
					continue;
				}

				subpass.depth = att;
				subpass.vkDepth
					.setAttachment(att.index) //
					.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);
			}
			else {
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
					.setAttachment(att.index) //
					.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
			}
		}


		for (auto& att : inputs) {
			subpass.inputs.emplace_back(att);

			att.Get().isInputAttachment = true;

			if (att.Get().state != Attachment::State::ShaderRead) {
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
				.setAttachment(att.index) //
				.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

			att.Get().state = Attachment::State::ShaderRead;
		}

		subpass.descr
			.setInputAttachments(subpass.vkInputs) //
			.setColorAttachments(subpass.vkColors);

		if (subpass.depth.parent != nullptr) {
			subpass.descr.setPDepthStencilAttachment(&subpass.vkDepth);
		}
	}


	void Generate();


	//
	//
	//
	//
	RFramebuffer CreateFramebuffer(uint32 width, uint32 height)
	{
		RFramebuffer framebuffer;

		for (int32 i = 0; auto& att : internalAttachments) {
			vk::ImageUsageFlags usageBits = att.isDepth ? vk::ImageUsageFlagBits::eDepthStencilAttachment
														: vk::ImageUsageFlagBits::eColorAttachment;
			usageBits |= vk::ImageUsageFlagBits::eSampled;

			if (att.isInputAttachment) {
				usageBits |= vk::ImageUsageFlagBits::eInputAttachment;
			}

			vk::ImageLayout initialLayout = att.isDepth ? vk::ImageLayout::eDepthStencilAttachmentOptimal
														: vk::ImageLayout::eColorAttachmentOptimal;

			usageBits |= att.additionalFlags;

			framebuffer.AddAttachment(width, height, att.descr.format, vk::ImageTiling::eOptimal,
				vk::ImageLayout::eUndefined, usageBits, vk::MemoryPropertyFlagBits::eDeviceLocal,
				"FramebufferAttchment: " + std::to_string(i), initialLayout);
		}
		// WIP: add external attachments

		framebuffer.Generate(*compatibleRenderPass);
		return framebuffer;
	}
};


} // namespace vl
