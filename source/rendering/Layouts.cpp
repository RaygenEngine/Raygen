#include "Layouts.h"

#include "rendering/passes/DepthmapPass.h"

struct AttachmentDeclaration {
};

AttachmentDeclaration rendererFinalAttachment = {};

namespace vl {
inline constexpr static std::array colorAttachments = {
	std::pair{ "GNormal", vk::Format::eR16G16B16A16Snorm },
	std::pair{ "GAlbedo", vk::Format::eR32G32B32A32Sfloat },
	std::pair{ "GSpecularColor", vk::Format::eR32G32B32A32Sfloat },
	std::pair{ "GEmissive", vk::Format::eR8G8B8A8Srgb },
	std::pair{ "GVelocity", vk::Format::eR32G32B32A32Sfloat },
	std::pair{ "GUVDrawIndex", vk::Format::eR32G32B32A32Sfloat },
};

/*
// GBuffer
layout(set = 0, binding = 0) uniform sampler2D g_DepthSampler;
layout(set = 0, binding = 1) uniform sampler2D g_NormalSampler;
layout(set = 0, binding = 2) uniform sampler2D g_AlbedoSampler;
layout(set = 0, binding = 3) uniform sampler2D g_SpecularSampler;
layout(set = 0, binding = 4) uniform sampler2D g_EmissiveSampler;
layout(set = 0, binding = 5) uniform sampler2D g_VelocitySampler;
layout(set = 0, binding = 6) uniform sampler2D g_UVDrawIndexSampler;

layout(set = 0, binding = 7) uniform sampler2D std_BrdfLut;

layout(set = 0, binding = 8) uniform sampler2D raster_DirectLightSampler;

layout(set = 0, binding = 9) uniform sampler2D raster_IBLminusMirrorReflectionsSampler;

layout(set = 0, binding = 10) uniform sampler2D ray_MirrorReflectionsSampler;

layout(set = 0, binding = 11) uniform sampler2D ray_AOSampler;

// Blend Rast + Ray
layout(set = 0, binding = 12) uniform sampler2D sceneColorSampler;
*/

void Layouts_::MakeRenderPassLayouts()
{
	using Att = RRenderPassLayout::Attachment;
	using AttRef = RRenderPassLayout::AttachmentRef;

	AttRef depthBuffer;

	// GBuffer Pass
	{
		std::vector<AttRef> gbufferAtts;

		depthBuffer = gbufferPassLayout.CreateAttachment("GDepth", Device->FindDepthFormat());
		gbufferAtts.emplace_back(depthBuffer);

		for (auto& [name, format] : colorAttachments) {
			auto att = gbufferPassLayout.CreateAttachment(name, format);
			gbufferPassLayout.AttachmentFinalLayout(att, vk::ImageLayout::eShaderReadOnlyOptimal);
			gbufferAtts.emplace_back(att);
		}

		gbufferPassLayout.AddSubpass({}, std::move(gbufferAtts)); // Write GBuffer

		gbufferPassLayout.AttachmentFinalLayout(depthBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);

		gbufferPassLayout.Generate();
	}

	AttRef rasterDirectAtt;
	// RasterDirectLight
	{
		rasterDirectAtt
			= rasterDirectLightPassLayout.CreateAttachment("RasterDirectLight", vk::Format::eR32G32B32A32Sfloat);
		rasterDirectLightPassLayout.AddSubpass({}, { rasterDirectAtt }); // Write Direct Lighting

		rasterDirectLightPassLayout.AttachmentFinalLayout(rasterDirectAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		rasterDirectLightPassLayout.Generate();
	}

	AttRef rasterIblAtt;
	// RasterDirectLight
	{
		rasterIblAtt = rasterIblPassLayout.CreateAttachment("RasterIbl", vk::Format::eR32G32B32A32Sfloat);
		rasterIblPassLayout.AddSubpass({}, { rasterIblAtt }); // Write Direct Lighting

		rasterIblPassLayout.AttachmentFinalLayout(rasterIblAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		rasterIblPassLayout.Generate();
	}


	// Svgf Pass. Semi special case as we constantly swap 2 "framebuffers" internally.
	{
		auto att = svgfPassLayout.CreateAttachment("SVGF Result", vk::Format::eR32G32B32A32Sfloat);

		svgfPassLayout.AddSubpass({}, { att });
		svgfPassLayout.AttachmentFinalLayout(att, vk::ImageLayout::eShaderReadOnlyOptimal);
		svgfPassLayout.Generate();
	}

	// Lightblend + PostProcess
	{
		auto renderOutAttachment
			= ptPassLayout.CreateAttachment("LightBlend+PostProcess", vk::Format::eR32G32B32A32Sfloat);

		ptPassLayout.AddSubpass({ depthBuffer }, { renderOutAttachment }); // LightBlend + Unlit Pass

		ptPassLayout.AttachmentFinalLayout(renderOutAttachment, vk::ImageLayout::eShaderReadOnlyOptimal);

		// Rest of post proc
		ptPassLayout.Generate();
	}
}


Layouts_::Layouts_()
{
	for (uint32 i = 0u; i < colorAttachments.size() + 7; ++i) {
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
		vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eRaygenKHR);
	singleSamplerDescLayout.Generate();

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


	depthRenderPass = DepthmapPass::CreateCompatibleRenderPass();


	storageImageArray6.AddBinding(vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eAll, 6u);
	storageImageArray6.Generate();


	MakeRenderPassLayouts();

	{
		auto colorAtt
			= singleFloatColorAttPassLayout.CreateAttachment("FloatColorAtt", vk::Format::eR32G32B32A32Sfloat);

		singleFloatColorAttPassLayout.AddSubpass({}, { colorAtt });

		singleFloatColorAttPassLayout.AttachmentFinalLayout(colorAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		singleFloatColorAttPassLayout.Generate();
	}
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
