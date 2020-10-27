#include "SceneReflprobe.h"

#include "rendering/util/WriteDescriptorSets.h"
#include "rendering/scene/Scene.h"

using namespace vl;

SceneReflprobe::SceneReflprobe()
	: SceneStruct(sizeof(Reflprobe_UBO))
{
	reflDescSet = Layouts->envmapLayout.AllocDescriptorSet();
	surroundingEnvSamplerDescSet = Layouts->cubemapLayout.AllocDescriptorSet();
	surroundingEnvStorageDescSet = Layouts->singleStorageImage.AllocDescriptorSet();

	ptcube_faceArrayDescSet = Layouts->storageImageArray6.AllocDescriptorSet();
}

void SceneReflprobe::ShouldResize()
{
	int32 resolution = std::pow(2, ubo.lodCount);

	if (resolution == surroundingEnv.extent.width) {
		return;
	}

	Device->waitIdle();

	surroundingEnv = RCubemap(resolution, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("SurrCube: WIP:reflprobenamehere"));

	irradiance = RCubemap(32, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("IrrCube: WIP:reflprobenamehere"));

	prefiltered = RCubemap(resolution, ubo.lodCount, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled | vk::ImageUsageFlagBits::eColorAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("PreCube: WIP:reflprobenamehere"));

	rvk::writeDescriptorImages(reflDescSet, 0u, { surroundingEnv.view(), irradiance.view(), prefiltered.view() });

	rvk::writeDescriptorImages(surroundingEnvSamplerDescSet, 0u, { surroundingEnv.view() });

	rvk::writeDescriptorImages(surroundingEnvStorageDescSet, 0u, { surroundingEnv.view() },
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	ptcube_faceViews = surroundingEnv.GetFaceViews();

	rvk::writeDescriptorImageArray(ptcube_faceArrayDescSet, 0u, vk::uniqueToRaw(ptcube_faceViews), nullptr,
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

	irr_faceViews = irradiance.GetFaceViews();

	// create framebuffers for each face
	for (uint32 i = 0; i < 6; ++i) {
		std::array attachments{ irr_faceViews[i].get() };

		vk::FramebufferCreateInfo createInfo{};
		createInfo
			.setRenderPass(Layouts->singleFloatColorAttPassLayout.compatibleRenderPass.get()) //
			.setAttachments(attachments)
			.setWidth(32)
			.setHeight(32)
			.setLayers(1);

		irr_framebuffer[i] = Device->createFramebufferUnique(createInfo);
	}


	///////////////////////////////
	pref_cubemapMips.clear();
	// create framebuffers for each lod/face
	for (uint32 mip = 0; mip < ubo.lodCount; ++mip) {

		pref_cubemapMips.emplace_back();
		pref_cubemapMips[mip].faceViews = prefiltered.GetFaceViews(mip);

		for (uint32 i = 0; i < 6; ++i) {

			// reisze framebuffer according to mip-level size.
			uint32 mipResolution = resolution / static_cast<uint32>(std::round((std::pow(2, mip))));


			std::array attachments{ pref_cubemapMips[mip].faceViews[i].get() };

			vk::FramebufferCreateInfo createInfo{};
			createInfo
				.setRenderPass(Layouts->singleFloatColorAttPassLayout.compatibleRenderPass.get()) //
				.setAttachments(attachments)
				.setWidth(mipResolution)
				.setHeight(mipResolution)
				.setLayers(1);

			pref_cubemapMips[mip].framebuffers[i] = Device->createFramebufferUnique(createInfo);
		}
	}

	shouldBuild.Set();
}
