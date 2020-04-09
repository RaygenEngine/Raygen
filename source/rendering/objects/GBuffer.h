#pragma once
#include "rendering/objects/Image2D.h"
#include "rendering/Device.h"

#include <vulkan/vulkan.hpp>

namespace vl {
enum GColorAttachment : uint32
{
	GPosition = 0,
	GNormal = 1,
	// rgb: albedo, a: opacity
	GAlbedo = 2,
	// r: metallic, g: roughness, b: occlusion, a: occlusion strength
	GSpecular = 3,
	GEmissive = 4,
	GDepth = 5,
	GCount = 6
};

struct GBuffer {

	inline constexpr static std::array<vk::Format, 5> colorAttachmentFormats = { vk::Format::eR8G8B8A8Unorm,
		vk::Format::eR8G8B8A8Unorm, vk::Format::eR8G8B8A8Srgb, vk::Format::eR8G8B8A8Unorm, vk::Format::eR8G8B8A8Srgb };
	inline constexpr static std::array<const char*, 6> attachmentNames
		= { "position", "normal", "albedo", "specular", "emissive", "depth" };

	std::array<UniquePtr<Image2D>, 6> attachments;

	GBuffer(uint32 width, uint32 height);

	void TransitionForAttachmentWrite(vk::CommandBuffer cmdBuffer);

	std::array<vk::ImageView, 6> GetViewsArray();

	Image2D* operator[](uint32 i) const { return attachments[i].get(); }
};
} // namespace vl
