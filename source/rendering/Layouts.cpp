#include "Layouts.h"

#include "rendering/Device.h"

struct AttachmentDeclaration {
};

AttachmentDeclaration rendererFinalAttachment = {};

namespace vl {

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

	// binding 0 sampler2D g_DepthSampler
	global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	// binding 1 sampler2D g_SNormalSampler
	// binding 2 sampler2D g_GNormalSampler
	// binding 3 sampler2D g_AlbedoSampler
	// binding 4 sampler2D g_SpecularSampler
	// binding 5 sampler2D g_EmissiveSampler
	// binding 6 sampler2D g_VelocitySampler
	// binding 7 sampler2D g_UVDrawIndexSampler
	for (uint32 i = 0u; i < PassLayouts_::gBufferColorAttachments.size(); ++i) {
		global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	}
	// binding 8 sampler2D directLightSampler
	global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	// binding 9 sampler2D indirectLightSampler
	global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	// binding 10 sampler2D ambientLightSampler
	global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	// binding 11 sampler2D std_BrdfLut
	global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	// binding 12 sampler2D _reserved0_
	global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	// binding 13 sampler2D _reserved1_
	global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	// binding 14 sampler2D _reserved2_
	global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	// binding 15 sampler2D sceneColorSampler
	global.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	// binding 16 ubo Viewer
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
