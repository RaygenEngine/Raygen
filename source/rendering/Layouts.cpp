#include "Layouts.h"

#include "rendering/Device.h"
#include "rendering/passes/DepthmapPass.h"
#include "rendering/passes/GbufferPass.h"
#include "rendering/passes/LightblendPass.h"
#include "rendering/passes/UnlitPass.h"

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
layout(set = 0, binding = 0) uniform Sampler2d g_DepthSampler;
layout(set = 0, binding = 1) uniform Sampler2d g_NormalSampler;
layout(set = 0, binding = 2) uniform Sampler2d g_AlbedoSampler;
layout(set = 0, binding = 3) uniform Sampler2d g_SpecularSampler;
layout(set = 0, binding = 4) uniform Sampler2d g_EmissiveSampler;
layout(set = 0, binding = 5) uniform Sampler2d g_VelocitySampler;
layout(set = 0, binding = 6) uniform sampler2D g_GUVDrawIndexSampler;


// Raster Direct
layout(set = 0, binding = 7) uniform Sampler2d rasterDirectSampler;

// RayTracing
layout(set = 0, binding = 8) uniform Sampler2d rtIndirectSampler;

// Blend Rast + Ray
layout(set = 0, binding = 9) uniform Sampler2d sceneColorSampler;
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
	// RasterDirect
	{
		rasterDirectAtt = rasterDirectPassLayout.CreateAttachment("RasterDirect", vk::Format::eR32G32B32A32Sfloat);
		rasterDirectPassLayout.AddSubpass({}, { rasterDirectAtt }); // Write Direct Lighting

		rasterDirectPassLayout.AttachmentFinalLayout(rasterDirectAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		rasterDirectPassLayout.Generate();
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

	for (uint32 i = 0u; i < colorAttachments.size() + 4; ++i) {
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


	stbuffer.AddBinding(eStorageBuffer, eRaygenKHR | eClosestHitKHR | eAnyHitKHR, 1024u, eVariableDescriptorCount);
	stbuffer.Generate();

	bufferAndSamplersDescLayout.AddBinding(eStorageBuffer, eRaygenKHR | eClosestHitKHR | eAnyHitKHR);

	bufferAndSamplersDescLayout.AddBinding(
		eCombinedImageSampler, eRaygenKHR | eClosestHitKHR | eAnyHitKHR, 1024u, eVariableDescriptorCount);

	bufferAndSamplersDescLayout.Generate();


	depthRenderPass = DepthmapPass::CreateCompatibleRenderPass();


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
