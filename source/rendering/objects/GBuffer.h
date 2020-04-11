#pragma once
#include "rendering/objects/Framebuffer.h"
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

class GBuffer {
public:
	inline constexpr static std::array<vk::Format, 5> colorAttachmentFormats
		= { vk::Format::eR32G32B32A32Sfloat, vk::Format::eR32G32B32A32Sfloat, vk::Format::eR8G8B8A8Srgb,
			  vk::Format::eR8G8B8A8Unorm, vk::Format::eR8G8B8A8Srgb };
	inline constexpr static std::array<const char*, 6> attachmentNames
		= { "position", "normal", "albedo", "specular", "emissive", "depth" };

private:
	Framebuffer m_framebuffer;
	vk::DescriptorSet m_descSet;

public:
	GBuffer(vk::RenderPass renderPass, uint32 width, uint32 height);

	void TransitionForAttachmentWrite(vk::CommandBuffer* cmdBuffer);

	[[nodiscard]] vk::DescriptorSet GetDescSet() const { return m_descSet; }
	[[nodiscard]] Framebuffer& GetFramebuffer() { return m_framebuffer; }

	void UpdateDescriptorSet();
};
} // namespace vl
