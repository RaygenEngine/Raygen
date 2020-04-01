#pragma once
#include "rendering/wrapper/Attachment.h"

#include <vulkan/vulkan.hpp>

namespace vl {

enum GColorAttachment : size_t
{
	Position = 0,
	Normal = 1,
	Albedo = 2,
	Specular = 3,
	Emissive = 4
};


struct GBuffer {

	UniquePtr<Attachment> position;
	UniquePtr<Attachment> normal;
	// rgb: albedo, a: opacity
	UniquePtr<Attachment> albedo;
	// r: metallic, g: roughness, b: occlusion, a: occlusion strength
	UniquePtr<Attachment> specular;
	UniquePtr<Attachment> emissive;
	UniquePtr<Attachment> depth;

	GBuffer(uint32 width, uint32 height);

	void TransitionForAttachmentWrite(vk::CommandBuffer cmdBuffer);

	std::array<vk::ImageView, 6> GetViewsArray();

	static vk::Format GetGBufferAttachmentFormat(size_t attachmentIndex)
	{
		switch (attachmentIndex) {
			case GColorAttachment::Position: return vk::Format::eR8G8B8A8Unorm;
			case GColorAttachment::Normal: return vk::Format::eR8G8B8A8Unorm;
			case GColorAttachment::Albedo: return vk::Format::eR8G8B8A8Srgb;
			case GColorAttachment::Specular: return vk::Format::eR8G8B8A8Unorm;
			case GColorAttachment::Emissive: return vk::Format::eR8G8B8A8Srgb;
		}
	}
};
} // namespace vl
