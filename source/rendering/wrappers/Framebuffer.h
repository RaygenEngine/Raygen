#pragma once
#include "rendering/wrappers/Image.h"

namespace vl {
struct RImageAttachment;
// DOC: wrapper for framebuffer where each attachment has equal dimensions and layers = 1
struct RFramebuffer {

	vk::Extent2D extent{};

	// Owned attachments
	std::vector<RImageAttachment> ownedAttachments{};
	std::vector<vk::ImageView> attachmentViews{};

	vk::UniqueDescriptorSetLayout setLayout;

	bool hasBeenGenerated{ false };

	void AddAttachment(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
		const std::string& name, vk::ImageLayout finalLayout);

	// void AddExistingAttachment(vk::ImageView imageView);
	void AddExistingAttachment(const RImageAttachment& attachment);

	void Generate(vk::RenderPass compatibleRenderPass);


	operator vk::Framebuffer() const { return handle.get(); }
	const RImageAttachment& operator[](size_t i) const { return ownedAttachments[i]; }

private:
	vk::UniqueFramebuffer handle;
};

} // namespace vl
