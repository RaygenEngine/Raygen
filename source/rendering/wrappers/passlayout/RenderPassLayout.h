#pragma once

#include "rendering/wrappers/Framebuffer.h"

namespace vl {


struct RRenderPassLayout {

	struct Attachment {
		enum class State
		{
			ShaderRead,
			Depth,
			Color,
		} state; // current subpass state.

		vk::ImageUsageFlags additionalFlags{};

		bool isDepth{ false };
		bool isInputAttachment{ false };
		vk::Format format{ vk::Format::eUndefined };
	};

	struct AttachmentRef {
		RRenderPassLayout* originalOwner{ nullptr };
		int32 originalIndex{ -1 };

		RRenderPassLayout* thisOwner{ nullptr };
		int32 thisIndex{ -1 };
		int32 firstUseIndex{ -1 };

		[[nodiscard]] Attachment& Get() const { return originalOwner->internalAttachments[originalIndex]; }

		[[nodiscard]] vk::AttachmentDescription& GetDescr() const
		{
			if (IsInternal()) {
				return originalOwner->internalAttachmentsDescr[originalIndex];
			}
			return thisOwner->externalAttachmentsDescr[thisIndex];
		}


		void UpdateState(Attachment::State state) { Get().state = state; }
		[[nodiscard]] bool IsDepth() const { return Get().isDepth; }
		[[nodiscard]] vk::Format GetFormat() const { return Get().format; }

	private:
		[[nodiscard]] bool IsInternal() const { return thisOwner == originalOwner && thisOwner != nullptr; }

	public:
		[[nodiscard]] bool IsSame(AttachmentRef other) const
		{
			return originalOwner == other.originalOwner && originalIndex == other.originalIndex;
		}

		[[nodiscard]] int32 GetAttachmentIndex() const
		{
			if (IsInternal()) {
				return originalIndex;
			}
			return int32(thisOwner->internalAttachments.size() + thisIndex);
		}
	};

private:
	// Create or get a reference for an external attachment as used in this render pass.
	// Returns the argument if attachment is already owned by this render pass.
	AttachmentRef UseExternal(AttachmentRef attachment, int32 firstUseIndex);
	bool IsExternal(AttachmentRef att) { return att.originalOwner != this; }

public:
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
	std::vector<vk::AttachmentDescription> internalAttachmentsDescr;
	std::vector<AttachmentRef> externalAttachments;
	std::vector<vk::AttachmentDescription> externalAttachmentsDescr;

	std::vector<Subpass> subpasses;


	[[nodiscard]] AttachmentRef CreateAttachment(vk::Format format, vk::ImageUsageFlags additionalUsageFlags = {})
	{
		auto& att = internalAttachments.emplace_back();
		auto& attDescr = internalAttachmentsDescr.emplace_back();

		AttachmentRef attRef = AttachmentRef{ this, int32(internalAttachments.size() - 1) };
		attRef.thisOwner = this;
		attRef.thisIndex = int32(internalAttachments.size() - 1);

		att.format = format;
		att.isDepth = rvk::isDepthFormat(format);
		att.additionalFlags = additionalUsageFlags;

		attDescr
			.setFormat(format) //
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(
				att.isDepth ? vk::ImageLayout::eDepthAttachmentOptimal : vk::ImageLayout::eColorAttachmentOptimal)
			.setFinalLayout(vk::ImageLayout::eUndefined);


		att.state = att.isDepth ? Attachment::State::Depth : Attachment::State::Color;

		return attRef;
	}


	void AddSubpass(std::vector<AttachmentRef>&& inputs, std::vector<AttachmentRef>&& outputs);
	void Generate();

	void TransitionAttachment(AttachmentRef att, vk::ImageLayout postRenderPassLayout);

public:
	//
	//
	//
	//
	[[nodiscard]] RFramebuffer CreateFramebuffer(
		uint32 width, uint32 height, std::vector<const RImageAttachment*> externalAttachmentInstances = {})
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

			framebuffer.AddAttachment(width, height, att.format, vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
				usageBits, vk::MemoryPropertyFlagBits::eDeviceLocal, "FramebufferAttchment: " + std::to_string(i),
				initialLayout);
		}

		CLOG_ABORT(externalAttachmentInstances.size() != externalAttachments.size(),
			"Incorrect number of attachments when generating framebuffer for render pass.");

		for (auto& att : externalAttachmentInstances) {
			framebuffer.AddExistingAttachment(*att);
		}

		framebuffer.Generate(*compatibleRenderPass);
		return framebuffer;
	}
};


} // namespace vl
