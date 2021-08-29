#pragma once
#include "rendering/wrappers/ImageView.h"

namespace vl {
struct RImage2D;
// DOC: wrapper for framebuffer where each attachment has equal dimensions and layers = 1
struct RFramebuffer {

	vk::Extent2D extent{};

	// Owned attachments
	std::vector<RImage2D> ownedAttachments{};
	std::vector<vk::ImageView> attachmentViews{};


	void AddAttachment(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties, const std::string& name,
		vk::ImageLayout finalLayout);

	// void AddExistingAttachment(vk::ImageView imageView);
	void AddExistingAttachment(const RImage2D& attachment);

	void Generate(vk::RenderPass compatibleRenderPass);

	[[nodiscard]] vk::Framebuffer handle() const { return uHandle.get(); }

	// TODO: differentiate between depth attachments and color attachments
	[[nodiscard]] const RImage2D& operator[](size_t i) const { return ownedAttachments[i]; }
	[[nodiscard]] const RImage2D& operator[](std::string_view name) const { return ownedAttachments[indices.at(name)]; }

	// auto begin() { return std::begin(attachments); }
	// auto begin() const { return std::begin(attachments); }
	// auto end() { return std::end(attachments); }
	// auto end() const { return std::end(attachments); }

private:
	vk::UniqueFramebuffer uHandle;

	std::unordered_map<std::string_view, size_t> indices{};

	bool hasBeenGenerated{ false };
};

} // namespace vl
