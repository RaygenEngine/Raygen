#include "pch.h"
#include "Layouts.h"

#include "rendering/passes/DepthmapPass.h"
#include "rendering/passes/GbufferPass.h"
#include "rendering/passes/UnlitPass.h"
#include "rendering/passes/LightblendPass.h"


struct AttachmentDeclaration {
};

AttachmentDeclaration rendererFinalAttachment = {};

namespace vl {


inline constexpr static std::array colorAttachmentFormats = { vk::Format::eR16G16B16A16Snorm,
	/*vk::Format::eR16G16B16A16Snorm,*/ vk::Format::eR8G8B8A8Srgb, vk::Format::eR8G8B8A8Unorm,
	vk::Format::eR8G8B8A8Srgb };
inline constexpr static std::array attachmentNames
	= { "Depth", "Normal", /*"GeomNormal", */ "BaseColor", "Surface", "Emissive" };

inline constexpr static size_t ColorAttachmentCount = colorAttachmentFormats.size();


/*
// GBuffer
layout(set = 0, binding = 0) uniform Sampler2d g_DepthSampler;
layout(set = 0, binding = 1) uniform Sampler2d g_NormalSampler;
layout(set = 0, binding = 2) uniform Sampler2d g_ColorSampler;
layout(set = 0, binding = 3) uniform Sampler2d g_MRROSampler; // Metallic Roughness Reflectance Occlusion
layout(set = 0, binding = 4) uniform Sampler2d g_EmissiveSampler;

// Raster Direct
layout(set = 0, binding = 5) uniform Sampler2d rasterDirectSampler;

// RayTracing
layout(set = 0, binding = 6) uniform Sampler2d rtIndirectSampler;

// Blend Rast + Ray
layout(set = 0, binding = 7) uniform Sampler2d sceneColorSampler;
*/

void Layouts_::MakeRenderPassLayouts()
{
	using Att = RRenderPassLayout::Attachment;
	using AttRef = RRenderPassLayout::AttachmentRef;

	AttRef depthBuffer;

	// GBuffer Pass
	{
		std::vector<AttRef> gbufferAtts;

		depthBuffer = gbufferPassLayout.CreateAttachment(Device->FindDepthFormat());
		gbufferAtts.emplace_back(depthBuffer);

		for (auto gbufferAttFormat : colorAttachmentFormats) {
			auto att = gbufferPassLayout.CreateAttachment(gbufferAttFormat);
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
		rasterDirectAtt = rasterDirectPassLayout.CreateAttachment(vk::Format::eR32G32B32A32Sfloat);
		rasterDirectPassLayout.AddSubpass({}, { rasterDirectAtt }); // Write Direct Lighting

		rasterDirectPassLayout.AttachmentFinalLayout(rasterDirectAtt, vk::ImageLayout::eShaderReadOnlyOptimal);

		rasterDirectPassLayout.Generate();
	}

	// Lightblend + PostProcess
	{
		auto renderOutAttachment = ptPassLayout.CreateAttachment(vk::Format::eR32G32B32A32Sfloat);

		ptPassLayout.AddSubpass({ depthBuffer }, { renderOutAttachment }); // LightBlend + Unlit Pass

		ptPassLayout.AttachmentFinalLayout(renderOutAttachment, vk::ImageLayout::eShaderReadOnlyOptimal);

		// Rest of post proc
		ptPassLayout.Generate();
	}
}


Layouts_::Layouts_()
{

	for (uint32 i = 0u; i < 8; ++i) {
		renderAttachmentsLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler,
			vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eRaygenKHR
				| vk::ShaderStageFlagBits::eClosestHitKHR);
	}
	renderAttachmentsLayout.Generate();


	// gltf material
	gltfMaterialDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);
	for (uint32 i = 0; i < 5u; ++i) {
		gltfMaterialDescLayout.AddBinding(
			vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	}
	gltfMaterialDescLayout.Generate();

	// single
	singleUboDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eAll); // WIP:
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


	// rt base
	singleStorageImage.AddBinding(vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eAll); // WIP: Fix all
	singleStorageImage.Generate();

	doubleStorageImage.AddBinding(vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eAll); // WIP: Fix all
	doubleStorageImage.AddBinding(vk::DescriptorType::eStorageImage, vk::ShaderStageFlagBits::eAll); // WIP: Fix all
	doubleStorageImage.Generate();

	// image debug
	imageDebugDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	imageDebugDescLayout.Generate();


	// WIP: Hardcoded size
	rtSceneDescLayout.AddBinding(
		vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eClosestHitKHR, 25 * 3);
	rtSceneDescLayout.AddBinding(
		vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR); // Vertex buffer
	rtSceneDescLayout.AddBinding(
		vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR); // Index buffer
	rtSceneDescLayout.AddBinding(
		vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR); // Index Offsets buffer
	rtSceneDescLayout.AddBinding(
		vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR); // Primitive Offsets buffer

	// Spotlights Buffer
	rtSceneDescLayout.AddBinding(vk::DescriptorType::eStorageBuffer, vk::ShaderStageFlagBits::eClosestHitKHR);

	// Shadowmaps
	rtSceneDescLayout.AddBinding(
		vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eClosestHitKHR, 16);


	rtSceneDescLayout.Generate();


	depthRenderPass = DepthmapPass::CreateCompatibleRenderPass();


	MakeRenderPassLayouts();
}
} // namespace vl
