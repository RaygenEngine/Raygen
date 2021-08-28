#include "Layouts.h"

#include "rendering/Device.h"

struct AttachmentDeclaration {
};

AttachmentDeclaration rendererFinalAttachment = {};

namespace vl {


/*
// GBuffer
layout(set = 0, binding = 0) uniform sampler2D g_DepthSampler;
layout(set = 0, binding = 1) uniform sampler2D g_SNormalSampler;
layout(set = 0, binding = 2) uniform sampler2D g_GNormalSampler;
layout(set = 0, binding = 3) uniform sampler2D g_AlbedoSampler;
layout(set = 0, binding = 4) uniform sampler2D g_SpecularSampler;
layout(set = 0, binding = 5) uniform sampler2D g_EmissiveSampler;
layout(set = 0, binding = 6) uniform sampler2D g_VelocitySampler;
layout(set = 0, binding = 7) uniform sampler2D g_UVDrawIndexSampler;

layout(set = 0, binding = 8) uniform sampler2D directLightSampler;
layout(set = 0, binding = 9) uniform sampler2D indirectLightSampler;

layout(set = 0, binding = 10) uniform sampler2D aoSampler;

layout(set = 0, binding = 11) uniform sampler2D std_BrdfLut;

layout(set = 0, binding = 12) uniform sampler2D _reserved0_;
layout(set = 0, binding = 13) uniform sampler2D _reserved1_;

layout(set = 0, binding = 14) uniform sampler2D mirrorSampler;

layout(set = 0, binding = 15) uniform sampler2D sceneColorSampler;
*/

PassLayouts_::PassLayouts_()
{
	using Att = RRenderPassLayout::Attachment;
	using AttRef = RRenderPassLayout::AttachmentRef;

	AttRef depthBuffer;

	// Main Pass
	{
		std::vector<AttRef> gbufferAtts;

		depthBuffer = main.CreateAttachment("GDepth", Device->FindDepthFormat());
		gbufferAtts.emplace_back(depthBuffer);

		for (auto& [name, format] : gBufferColorAttachments) {
			auto att = main.CreateAttachment(name, format);
			main.AttachmentFinalLayout(att, vk::ImageLayout::eShaderReadOnlyOptimal);
			gbufferAtts.emplace_back(att);
		}

		AttRef directAtt = main.CreateAttachment("DirectLight", vk::Format::eR32G32B32A32Sfloat);
		AttRef indirectAtt = main.CreateAttachment("IndirectLight", vk::Format::eR32G32B32A32Sfloat);

		main.AddSubpass({}, std::vector{ gbufferAtts });              // Write GBuffer
		main.AddSubpass(std::vector{ gbufferAtts }, { directAtt });   // Write DirectLights
		main.AddSubpass(std::vector{ gbufferAtts }, { indirectAtt }); // Write IndirectLights

		main.AttachmentFinalLayout(depthBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
		main.AttachmentFinalLayout(directAtt, vk::ImageLayout::eShaderReadOnlyOptimal);
		main.AttachmentFinalLayout(indirectAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		main.Generate();
	}

	AttRef ptColorAtt;
	{
		ptColorAtt = pt.CreateAttachment("Pt color", vk::Format::eR32G32B32A32Sfloat);

		pt.AddSubpass({}, { ptColorAtt });

		pt.AttachmentFinalLayout(ptColorAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		pt.Generate();
	}

	{
		unlit.AddSubpass({}, { depthBuffer, ptColorAtt }); // Unlit Pass

		unlit.AttachmentFinalLayout(ptColorAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		unlit.Generate();
	}

	// Shadow Pass
	{
		AttRef shadowAtt = shadow.CreateAttachment("Shadowmap", Device->FindDepthFormat());

		shadow.AddSubpass({}, { shadowAtt });

		shadow.AttachmentFinalLayout(shadowAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		shadow.Generate();
	}

	{
		auto colorAtt = secondary.CreateAttachment("Ambient", vk::Format::eR32G32B32A32Sfloat);

		secondary.AddSubpass({}, { colorAtt });

		secondary.AttachmentFinalLayout(colorAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		secondary.Generate();
	}

	{
		auto colorAtt = singleFloatColorAtt.CreateAttachment("FloatColorAtt", vk::Format::eR32G32B32A32Sfloat);

		singleFloatColorAtt.AddSubpass({}, { colorAtt });

		singleFloatColorAtt.AttachmentFinalLayout(colorAtt, vk::ImageLayout::eShaderReadOnlyOptimal);


		singleFloatColorAtt.Generate();
	}


	// Svgf Pass. Semi special case as we constantly swap 2 "framebuffers" internally.
	{
		auto att = svgf.CreateAttachment("SVGF Result", vk::Format::eR32G32B32A32Sfloat);

		svgf.AddSubpass({}, std::vector{ att });
		svgf.AttachmentFinalLayout(att, vk::ImageLayout::eShaderReadOnlyOptimal);
		svgf.Generate();
	}
}


DescriptorLayouts_::DescriptorLayouts_()
{
	using enum vk::ShaderStageFlagBits;
	using enum vk::DescriptorType;
	using enum vk::DescriptorBindingFlagBits;

	// TODO: + 8, gDepth + rest
	for (uint32 i = 0u; i < PassLayouts_::gBufferColorAttachments.size() + 9; ++i) {
		global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	}
	global.AddBinding(eUniformBuffer, eFragment | eVertex | eCompute | eRaygenKHR);
	global.Generate();

	joints.AddBinding(eStorageBuffer, eVertex);
	joints.Generate();

	accelerationStructure.AddBinding(eAccelerationStructureKHR, eFragment | eRaygenKHR | eClosestHitKHR);
	accelerationStructure.Generate();

	_1storageBuffer_1024samplerImage.AddBinding(eStorageBuffer, eRaygenKHR | eClosestHitKHR | eAnyHitKHR);
	_1storageBuffer_1024samplerImage.AddBinding(
		eCombinedImageSampler, eRaygenKHR | eClosestHitKHR | eAnyHitKHR, 1024u, eVariableDescriptorCount);
	_1storageBuffer_1024samplerImage.Generate();

	_1imageSampler_2storageImage.AddBinding(eCombinedImageSampler, eCompute);
	_1imageSampler_2storageImage.AddBinding(eStorageImage, eCompute);
	_1imageSampler_2storageImage.AddBinding(eStorageImage, eCompute);
	_1imageSampler_2storageImage.Generate();

	_1imageSamplerFragmentOnly.AddBinding(eCombinedImageSampler, eFragment);
	_1imageSamplerFragmentOnly.Generate();
}

} // namespace vl
