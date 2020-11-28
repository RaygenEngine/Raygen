#pragma once
#include "rendering/wrappers/ImageView.h"

namespace vl {
struct RImageAttachment;
// DOC: wrapper for framebuffer where each attachment has equal dimensions and layers = 1
struct RFramebuffer {

	vk::Extent2D extent{};

	// Owned attachments
	std::vector<RImageAttachment> ownedAttachments{};
	std::vector<vk::ImageView> attachmentViews{};

	void AddAttachment(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
		const std::string& name, vk::ImageLayout finalLayout);

	// void AddExistingAttachment(vk::ImageView imageView);
	void AddExistingAttachment(const RImageAttachment& attachment);

	void Generate(vk::RenderPass compatibleRenderPass);

	[[nodiscard]] vk::Framebuffer handle() const { return uHandle.get(); }

	// WIP: differentiate between depth attachments and color attachments
	[[nodiscard]] const RImageAttachment& operator[](size_t i) const { return ownedAttachments[i]; }

	// auto begin() { return std::begin(attachments); }
	// auto begin() const { return std::begin(attachments); }
	// auto end() { return std::end(attachments); }
	// auto end() const { return std::end(attachments); }

private:
	vk::UniqueFramebuffer uHandle;


	bool hasBeenGenerated{ false };
};

} // namespace vl
