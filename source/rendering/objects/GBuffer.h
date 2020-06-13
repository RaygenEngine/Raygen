#pragma once
#include "rendering/Device.h"
#include "rendering/objects/RImageAttachment.h"
#include "rendering/passes/GBufferPass.h"

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
	vk::UniqueFramebuffer m_framebuffer;
	std::array<UniquePtr<RImageAttachment>, 6> m_attachments;

	vk::DescriptorSet m_descSet;

	vk::UniquePipeline m_pipeline;

	vk::Extent2D m_extent;

	void MakePipeline(GBufferPass* passInfo);

public:
	vk::UniquePipeline wip_CreatePipeline(vk::PipelineLayout pipelineLayout, vk::RenderPass renderPass,
		std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages);

public:
	GBuffer(GBufferPass* passInfo, uint32 width, uint32 height);

	void TransitionForWrite(vk::CommandBuffer* cmdBuffer);

	[[nodiscard]] vk::DescriptorSet GetDescSet() const { return m_descSet; }
	[[nodiscard]] vk::Framebuffer GetFramebuffer() const { return m_framebuffer.get(); }
	[[nodiscard]] vk::Extent2D GetExtent() const { return m_extent; }
	[[nodiscard]] vk::Pipeline GetPipeline() const { return m_pipeline.get(); }

	[[nodiscard]] RImageAttachment* operator[](uint32 i) const { return m_attachments[i].get(); }
};
} // namespace vl
