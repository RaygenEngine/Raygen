#include "SceneIrradianceGrid.h"

#include "rendering/util/WriteDescriptorSets.h"
#include "rendering/scene/Scene.h"

using namespace vl;

SceneIrradianceGrid::SceneIrradianceGrid()
	: SceneStruct(sizeof(IrradianceGrid_UBO))
{
	Device->waitIdle();

	gridDescSet = Layouts->cubemapArray1024.AllocDescriptorSet();

	std::vector<vk::ImageView> views;
	for (uint32 i = 0u; i < IRRGRID_PROBE_COUNT; ++i) {

		probes[i].surroundingEnvSamplerDescSet = Layouts->cubemapLayout.AllocDescriptorSet();
		probes[i].surroundingEnvStorageDescSet = Layouts->singleStorageImage.AllocDescriptorSet();

		probes[i].ptcube_faceArrayDescSet = Layouts->storageImageArray6.AllocDescriptorSet();


		probes[i].surroundingEnv = RCubemap(64u, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
			vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("SurrCube: WIP:irradiancegrid"));

		probes[i].irradiance
			= RCubemap(64u, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
				vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled
					| vk::ImageUsageFlagBits::eColorAttachment,
				vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("IrrCube: WIP:irradiancegrid"));

		rvk::writeDescriptorImages(probes[i].surroundingEnvSamplerDescSet, 0u, { probes[i].surroundingEnv.view() });
		rvk::writeDescriptorImages(probes[i].surroundingEnvStorageDescSet, 0u, { probes[i].surroundingEnv.view() },
			vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

		probes[i].ptcube_faceViews = probes[i].surroundingEnv.GetFaceViews();

		rvk::writeDescriptorImageArray(probes[i].ptcube_faceArrayDescSet, 0u,
			vk::uniqueToRaw(probes[i].ptcube_faceViews), nullptr, vk::DescriptorType::eStorageImage,
			vk::ImageLayout::eGeneral);

		probes[i].irr_faceViews = probes[i].irradiance.GetFaceViews();

		// create framebuffers for each face
		probes[i].irr_framebuffer.clear();
		for (uint32 f = 0; f < 6; ++f) {
			std::array attachments{ probes[i].irr_faceViews[f].get() };

			vk::FramebufferCreateInfo createInfo{};
			createInfo
				.setRenderPass(Layouts->singleFloatColorAttPassLayout.compatibleRenderPass.get()) //
				.setAttachments(attachments)
				.setWidth(probes[i].irradiance.extent.width)
				.setHeight(probes[i].irradiance.extent.width)
				.setLayers(1);

			probes[i].irr_framebuffer.emplace_back();
			probes[i].irr_framebuffer[f] = Device->createFramebufferUnique(createInfo);
		}

		views.push_back(probes[i].irradiance.view());
	}

	rvk::writeDescriptorImageArray(gridDescSet, 0u, std::move(views));

	shouldBuild.Set();
}
