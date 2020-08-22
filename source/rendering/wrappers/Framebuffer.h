#pragma once
#include "rendering/wrappers/Image.h"

namespace vl {
struct RImageAttachment;
// DOC: wrapper for framebuffer where each attachment has equal dimensions and layers = 1
struct RFramebuffer {

	vk::Extent2D extent{};

	void AddAttachment(uint32 width, uint32 height, vk::Format format, vk::ImageTiling tiling,
		vk::ImageLayout initialLayout, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties,
		const std::string& name, vk::ImageLayout finalLayout);
	void Generate(vk::RenderPass compatibleRenderPass);

	[[nodiscard]] vk::Framebuffer handle() const { return uHandle.get(); }
	[[nodiscard]] vk::DescriptorSetLayout setLayout() { return uSetLayout.get(); }

	[[nodiscard]] const RImageAttachment& operator[](size_t i) const { return attachments[i]; }

	auto begin() { return std::begin(attachments); }
	auto begin() const { return std::begin(attachments); }
	auto end() { return std::end(attachments); }
	auto end() const { return std::end(attachments); }

private:
	vk::UniqueFramebuffer uHandle;
	vk::UniqueDescriptorSetLayout uSetLayout;

	std::vector<RImageAttachment> attachments{};

	bool hasBeenGenerated{ false };
};
} // namespace vl
