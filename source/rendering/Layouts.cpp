#include "pch.h"
#include "Layouts.h"

#include "rendering/passes/DepthmapPass.h"
#include "rendering/passes/GbufferPass.h"
#include "rendering/passes/UnlitPass.h"
#include "rendering/passes/LightblendPass.h"
#include "rendering/structures/GBuffer.h"

namespace vl {


void Layouts_::MakeRenderPassLayouts()
{
	using Att = RRenderPassLayout::Attachment;
	using AttRef = RRenderPassLayout::AttachmentRef;

	AttRef depthBuffer;

	// GBuffer Pass
	{
		std::vector<AttRef> gbufferAtts;

		for (auto gbufferAttFormat : GBuffer::colorAttachmentFormats) {
			auto att = gbufferPassLayout.CreateAttachment(gbufferAttFormat);
			gbufferPassLayout.AttachmentFinalLayout(att, vk::ImageLayout::eShaderReadOnlyOptimal);
			gbufferAtts.emplace_back(att);
		}

		depthBuffer = gbufferPassLayout.CreateAttachment(Device->FindDepthFormat());
		gbufferAtts.emplace_back(depthBuffer);

		gbufferPassLayout.AddSubpass({}, std::move(gbufferAtts));

		gbufferPassLayout.AttachmentFinalLayout(depthBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);

		gbufferPassLayout.Generate();
	}


	// Post Process Pass
	{
		RRenderPassLayout& layout = ptPassLayout;


		auto lights = layout.CreateAttachment(vk::Format::eR32G32B32A32Sfloat);
		auto postprocOut = layout.CreateAttachment(vk::Format::eR32G32B32A32Sfloat, vk::ImageUsageFlagBits::eStorage);

		layout.AddSubpass({}, { lights });
		layout.AddSubpass({ depthBuffer, lights }, { postprocOut });


		layout.AttachmentFinalLayout(lights, vk::ImageLayout::eShaderReadOnlyOptimal);
		layout.AttachmentFinalLayout(postprocOut, vk::ImageLayout::eShaderReadOnlyOptimal);

		layout.AttachmentFinalLayout(depthBuffer, vk::ImageLayout::eShaderReadOnlyOptimal);

		layout.Generate();
	}
}


Layouts_::Layouts_()
{
	// gbuffer
	for (uint32 i = 0u; i < GCount; ++i) {
		gbufferDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler,
			vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eRaygenKHR);
	}
	gbufferDescLayout.Generate();

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
	rtSceneDescLayout.Generate();


	depthRenderPass = DepthmapPass::CreateCompatibleRenderPass();

	gbufferPass = GbufferPass::CreateCompatibleRenderPass();

	lightblendPass = LightblendPass::CreateCompatibleRenderPass();

	MakeRenderPassLayouts();
}
} // namespace vl
