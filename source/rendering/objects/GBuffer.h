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

// in case of depth this isnt constexpr
inline constexpr vk::Format GetGAttachmentFormat(uint32 i)
{
	switch (i) {
		case GPosition: return vk::Format::eR8G8B8A8Unorm;
		case GNormal: return vk::Format::eR8G8B8A8Unorm;
		case GAlbedo: return vk::Format::eR8G8B8A8Srgb;
		case GSpecular: return vk::Format::eR8G8B8A8Unorm;
		case GEmissive: return vk::Format::eR8G8B8A8Srgb;
		case GDepth: return Device->pd->FindDepthFormat();
	}
}

inline constexpr const char* GetGAttachmentName(uint32 i)
{
	switch (i) {
		case GPosition: return "position";
		case GNormal: return "normal";
		case GAlbedo: return "albedo";
		case GSpecular: return "specular";
		case GEmissive: return "emissive";
		case GDepth: return "depth";
	}
}

struct GBuffer {

	std::array<UniquePtr<Image2D>, 6> attachments;

	GBuffer(uint32 width, uint32 height);

	void TransitionForAttachmentWrite(vk::CommandBuffer cmdBuffer);

	std::array<vk::ImageView, 6> GetViewsArray();

	Image2D* operator[](int32 i) const { return attachments[i].get(); }
};
} // namespace vl
