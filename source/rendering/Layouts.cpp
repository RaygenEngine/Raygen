#include "Layouts.h"

#include "rendering/passes/geometry/DepthmapPass.h"

struct AttachmentDeclaration {
};

AttachmentDeclaration rendererFinalAttachment = {};

namespace vl {


/*
// GBuffer
layout(set = 0, binding = 0) uniform sampler2D g_DepthSampler;
layout(set = 0, binding = 1) uniform sampler2D g_NormalSampler;
layout(set = 0, binding = 2) uniform sampler2D g_AlbedoSampler;
layout(set = 0, binding = 3) uniform sampler2D g_SpecularSampler;
layout(set = 0, binding = 4) uniform sampler2D g_EmissiveSampler;
layout(set = 0, binding = 5) uniform sampler2D g_VelocitySampler;
layout(set = 0, binding = 6) uniform sampler2D g_UVDrawIndexSampler;

layout(set = 0, binding = 7) uniform sampler2D directLightSampler;

layout(set = 0, binding = 8) uniform sampler2D indirectLightSampler;

layout(set = 0, binding = 9) uniform sampler2D AoSampler;

layout(set = 0, binding = 10) uniform sampler2D std_BrdfLut;

layout(set = 0, binding = 11) uniform sampler2D indirectRaytracedSpecular;

// Blend
layout(set = 0, binding = 12) uniform sampler2D sceneColorSampler;
*/

void Layouts_::MakeRenderPassLayouts()
{
	using Att = RRenderPassLayout::Attachment;
	using AttRef = RRenderPassLayout::AttachmentRef;

	AttRef depthBuffer;

	// Main Pass
	{
		std::vector<AttRef> gbufferAtts;

		depthBuffer = mainPassLayout.CreateAttachment("GDepth", Device->FindDepthFormat());
		gbufferAtts.emplace_back(depthBuffer);

		for (auto& [name, format] : gBufferColorAttachments) {
			auto att = mainPassLayout.CreateAttachment(name, format);
			mainPassLayout.AttachmentFinalLayout(att, vk::ImageLayout::eShaderReadOnlyOptimal);
			gbufferAtts.emplace_back(att);
		}

		AttRef directAtt = mainPassLayout.CreateAttachment("DirectLight", vk::Format::eR32G32B32A32Sfloat);
		AttRef indirectAtt = mainPassLayout.CreateAttachment("Indirect", vk::Format::eR32G32B32A32Sfloat);

		mainPassLayout.AddSubpass({}, std::vector{ gbufferAtts });              // Write GBuffer
		mainPassLayout.AddSubpass(std::vector{ gbufferAtts }, { directAtt });   // Write DirectLights
		mainPassLayout.AddSubpass(std::vector{ gbufferAtts }, { indirectAtt }); // Write Indirect


		mainPassLayout.AttachmentFinalLayout(depthBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);
		mainPassLayout.AttachmentFinalLayout(directAtt, vk::ImageLayout::eShaderReadOnlyOptimal);
		mainPassLayout.AttachmentFinalLayout(indirectAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		mainPassLayout.Generate();
	}

	// Lightblend + PostProcess
	{
		auto renderOutAttachment
			= ptPassLayout.CreateAttachment("LightBlend+PostProcess", vk::Format::eR32G32B32A32Sfloat);

		ptPassLayout.AddSubpass({}, { depthBuffer, renderOutAttachment }); // LightBlend + Unlit Pass

		ptPassLayout.AttachmentFinalLayout(renderOutAttachment, vk::ImageLayout::eShaderReadOnlyOptimal);

		// Rest of post proc
		ptPassLayout.Generate();
	}

	// Shadow Pass
	{
		AttRef shadowAtt = shadowPassLayout.CreateAttachment("Shadowmap", Device->FindDepthFormat());

		shadowPassLayout.AddSubpass({}, { shadowAtt });

		shadowPassLayout.AttachmentFinalLayout(shadowAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		shadowPassLayout.Generate();
	}

	{
		auto colorAtt = secondaryPassLayout.CreateAttachment("Ambient", vk::Format::eR32G32B32A32Sfloat);

		secondaryPassLayout.AddSubpass({}, { colorAtt });

		secondaryPassLayout.AttachmentFinalLayout(colorAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		secondaryPassLayout.Generate();
	}

	{
		auto colorAtt
			= singleFloatColorAttPassLayout.CreateAttachment("FloatColorAtt", vk::Format::eR32G32B32A32Sfloat);

		singleFloatColorAttPassLayout.AddSubpass({}, { colorAtt });

		singleFloatColorAttPassLayout.AttachmentFinalLayout(colorAtt, vk::ImageLayout::eShaderReadOnlyOptimal);


		singleFloatColorAttPassLayout.Generate();
	}


	// Svgf Pass. Semi special case as we constantly swap 2 "framebuffers" internally.
	{
		auto att = svgfPassLayout.CreateAttachment("SVGF Result", vk::Format::eR32G32B32A32Sfloat);

		svgfPassLayout.AddSubpass({}, std::vector{ att });
		svgfPassLayout.AttachmentFinalLayout(att, vk::ImageLayout::eShaderReadOnlyOptimal);
		svgfPassLayout.Generate();
	}
}


Layouts_::Layouts_()
{
	// WIP: + 7
	for (uint32 i = 0u; i < gBufferColorAttachments.size() + 7; ++i) {
		renderAttachmentsLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler,
			vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eRaygenKHR
				| vk::ShaderStageFlagBits::eClosestHitKHR);
	}
	renderAttachmentsLayout.Generate();

	using enum vk::ShaderStageFlagBits;
	using enum vk::DescriptorType;
	using enum vk::DescriptorBindingFlagBits;

	// gltf material
	gltfMaterialDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	for (uint32 i = 0; i < 5u; ++i) {
		gltfMaterialDescLayout.AddBinding(
			vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	}
	gltfMaterialDescLayout.Generate();

	// single
	singleUboDescLayout.AddBinding(
		vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll); // CHECK: fix shader stage flags
	singleUboDescLayout.Generate();

	// joints
	jointsDescLayout.AddBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eVertex);
	jointsDescLayout.Generate();

	// single sampler
	singleSamplerDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler,
		vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eCompute);
	singleSamplerDescLayout.Generate();

	singleSamplerFragOnlyLayout.AddBinding(
		vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	singleSamplerFragOnlyLayout.Generate();


	// cubemap
	cubemapLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	cubemapLayout.Generate();

	// evnmap
	envmapLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	envmapLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	envmapLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	envmapLayout.Generate();

	// accel
	accelLayout.AddBinding(vk::DescriptorType::eAccelerationStructureKHR,
		vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eRaygenKHR
			| vk::ShaderStageFlagBits::eClosestHitKHR);
	accelLayout.Generate();

	// rt
	rtTriangleGeometry.AddBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
	rtTriangleGeometry.AddBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);
	rtTriangleGeometry.Generate();


	// image debug
	imageDebugDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	imageDebugDescLayout.Generate();


	singleStorageBuffer.AddBinding(eStorageBuffer, eRaygenKHR | eClosestHitKHR | eAnyHitKHR);
	singleStorageBuffer.Generate();

	bufferAndSamplersDescLayout.AddBinding(eStorageBuffer, eRaygenKHR | eClosestHitKHR | eAnyHitKHR);

	bufferAndSamplersDescLayout.AddBinding(
		eCombinedImageSampler, eRaygenKHR | eClosestHitKHR | eAnyHitKHR, 1024u, eVariableDescriptorCount);

	bufferAndSamplersDescLayout.Generate();

	storageImageArray6.AddBinding(vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eAll, 6u);
	storageImageArray6.Generate();

	storageImageArray10.AddBinding(vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eAll, 10u);
	storageImageArray10.Generate();

	cubemapArray.AddBinding(eCombinedImageSampler, eFragment | eRaygenKHR | eClosestHitKHR | eCompute);
	cubemapArray.Generate();

	cubemapArrayStorage.AddBinding(eStorageImage, eRaygenKHR | eCompute);
	cubemapArrayStorage.Generate();

	cubemapArray6.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 6u);
	cubemapArray6.Generate();

	cubemapArray64.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 64u);
	cubemapArray64.Generate();

	cubemapArray1024.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment, 1024u);
	cubemapArray1024.Generate();


	MakeRenderPassLayouts();
}

RDescriptorSetLayout Layouts_::GenerateStorageImageDescSet(size_t Count)
{
	RDescriptorSetLayout descLayout;
	for (size_t i = 0; i < Count; i++) {
		descLayout.AddBinding(vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eAll);
	}
	descLayout.Generate();
	return descLayout;
}
} // namespace vl
