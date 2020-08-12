#pragma once
#include "rendering/wrappers/RImage.h"

namespace vl {
enum GColorAttachment : uint32
{
	GNormal = 0,
	// rgb: color, a: opacity
	GBaseColor = 1,
	// r: metallic, g: roughness, b: reflectance, a: occlusion
	GSurface = 2,
	GEmissive = 3,
	GDepth = 4,
	GCount
};

struct RGbuffer {

	inline constexpr static std::array colorAttachmentFormats = { vk::Format::eR16G16B16A16Snorm,
		vk::Format::eR8G8B8A8Srgb, vk::Format::eR8G8B8A8Unorm, vk::Format::eR8G8B8A8Srgb };
	inline constexpr static std::array attachmentNames = { "Normal", "BaseColor", "Surface", "Emissive", "Depth" };
	inline constexpr static size_t ColorAttachmentCount = colorAttachmentFormats.size();

	vk::UniqueFramebuffer framebuffer;
	std::array<RImageAttachment, GCount> attachments{};

	vk::DescriptorSet descSet;

	RGbuffer() = default;
	RGbuffer(uint32 width, uint32 height);
};
} // namespace vl
