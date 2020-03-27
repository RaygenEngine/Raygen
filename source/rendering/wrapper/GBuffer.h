#pragma once
#include "rendering/wrapper/Attachment.h"

#include <vulkan/vulkan.hpp>

namespace vl {
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
};
} // namespace vl
