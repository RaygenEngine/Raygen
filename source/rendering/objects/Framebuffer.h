#pragma once
#include "rendering/objects/ImageAttachment.h"
#include "rendering/Device.h"

#include <vulkan/vulkan.hpp>

namespace vl {
class Framebuffer {


	vk::UniqueFramebuffer m_handle;
	std::vector<UniquePtr<ImageAttachment>> m_attachments;

	bool m_hasBeenGenerated{ false };

public:
	void AddAttachment(UniquePtr<ImageAttachment> attachment);
	void Generate(vk::RenderPass renderPass);

	[[nodiscard]] ImageAttachment* operator[](uint32 i) const { return m_attachments[i].get(); }

	[[nodiscard]] operator vk::Framebuffer() const { return m_handle.get(); }

	[[nodiscard]] uint32 GetWidth() const { return m_attachments.back()->GetExtent().width; }
	[[nodiscard]] uint32 GetHeight() const { return m_attachments.back()->GetExtent().height; }

	[[nodiscard]] std::vector<UniquePtr<ImageAttachment>>& GetAttachments() { return m_attachments; }

	// from shader read
	void TransitionForAttachmentWrite(vk::CommandBuffer* cmdBuffer);

	bool HasBeenGenerated() const { return m_hasBeenGenerated; }
};
} // namespace vl
