#pragma once

#include "rendering/wrappers/Framebuffer.h"
#include "rendering/wrappers/DescriptorSetLayout.h"

namespace vl {
struct RRenderPassLayout;

struct RenderingPassInstance {
	RFramebuffer framebuffer;
	vk::DescriptorSet internalDescSet{ nullptr };


	RRenderPassLayout* parent{ nullptr };

	void TransitionFramebufferForWrite(vk::CommandBuffer cmdBuffer);
	vk::RenderPass GetRenderPass() const;

private:
	friend struct RRenderPassLayout;
	uint32 parentPassIndex{ UINT_MAX };
};

// NOTE: this object is assumed to never move by the generated subobjects (aka RenderingPassInstance).
struct RRenderPassLayout {

	RRenderPassLayout(RRenderPassLayout const&) = delete;
	RRenderPassLayout(RRenderPassLayout&&) = delete;
	RRenderPassLayout& operator=(RRenderPassLayout const&) = delete;
	RRenderPassLayout& operator=(RRenderPassLayout&&) = delete;


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

	[[nodiscard]] bool IsExternal(AttachmentRef att) { return att.originalOwner != this; }

	[[nodiscard]] std::optional<AttachmentRef> GetInternalRefOf(AttachmentRef attachment);

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

	RRenderPassLayout(const std::string& inName = {});
	std::string name{};
	uint32 uidIndex{ UINT_MAX };

	vk::UniqueRenderPass compatibleRenderPass;

	std::vector<vk::SubpassDependency> subpassDependencies;

	std::vector<Attachment> internalAttachments;
	std::vector<vk::AttachmentDescription> internalAttachmentsDescr;
	std::vector<AttachmentRef> externalAttachments;
	std::vector<vk::AttachmentDescription> externalAttachmentsDescr;

	std::vector<Subpass> subpasses;

	// Contains input attachments for next subpasses
	RDescriptorSetLayout internalDescLayout;
	std::vector<AttachmentRef> internalInputAttachmentOrder;


	// Preperation Interface
	[[nodiscard]] AttachmentRef CreateAttachment(vk::Format format, vk::ImageUsageFlags additionalUsageFlags = {});
	void AttachmentFinalLayout(AttachmentRef att, vk::ImageLayout postRenderPassLayout);


	void AddSubpass(std::vector<AttachmentRef>&& inputs, std::vector<AttachmentRef>&& outputs);
	void Generate();

	[[nodiscard]] RenderingPassInstance CreatePassInstance(
		uint32 width, uint32 height, std::vector<const RImageAttachment*> externalAttachmentInstances = {});
};


} // namespace vl
