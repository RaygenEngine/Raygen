#pragma once
#include "renderer/wrapper/Attachment.h"

#include <vulkan/vulkan.hpp>

struct GBuffer {

	std::unique_ptr<Attachment> position;
	std::unique_ptr<Attachment> normal;
	// rgb: albedo, a: opacity
	std::unique_ptr<Attachment> albedo;
	// r: metallic, g: roughness, b: occlusion, a: occlusion strength
	std::unique_ptr<Attachment> specular;
	std::unique_ptr<Attachment> emissive;
	std::unique_ptr<Attachment> depth;

	GBuffer(uint32 width, uint32 height);

	void TransitionForAttachmentWrite(vk::CommandBuffer cmdBuffer);

	std::array<vk::ImageView, 6> GetViewsArray();
};
