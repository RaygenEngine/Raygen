#include "SceneIrradianceGrid.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/scene/Scene.h"
#include "rendering/util/WriteDescriptorSets.h"


using namespace vl;

void SceneIrradianceGrid::Allocate()
{
	Device->waitIdle();

	gridDescSet = Layouts->dynamicSamplerArray.AllocDescriptorSet(ubo.width * ubo.height * ubo.depth);

	ubo.builtCount = 0;

	for (int32 x = 0; x < ubo.width; ++x) {
		for (int32 y = 0; y < ubo.height; ++y) {
			for (int32 z = 0; z < ubo.depth; ++z) {

				ubo.builtCount++;

				int32 i = 0;
				i += x;
				i += y * ubo.width;
				i += z * ubo.width * ubo.height;

				probes[i].surroundingEnvSamplerDescSet = Layouts->cubemapLayout.AllocDescriptorSet();
				probes[i].surroundingEnvStorageDescSet = Layouts->singleStorageImage.AllocDescriptorSet();

				probes[i].ptcube_faceArrayDescSet = Layouts->storageImageArray6.AllocDescriptorSet();


				probes[i].surroundingEnv = RCubemap(32u, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
					vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled,
					vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("SurrCube: CHECK:irradiancegrid"));

				probes[i].irradiance = RCubemap(32u, 1u, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
					vk::ImageLayout::eUndefined,
					vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eSampled
						| vk::ImageUsageFlagBits::eColorAttachment,
					vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("IrrCube: CHECK:irradiancegrid"));

				rvk::writeDescriptorImages(
					probes[i].surroundingEnvSamplerDescSet, 0u, { probes[i].surroundingEnv.view() });
				rvk::writeDescriptorImages(probes[i].surroundingEnvStorageDescSet, 0u,
					{ probes[i].surroundingEnv.view() }, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

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


				vk::DescriptorImageInfo viewInfo{};
				viewInfo
					.setImageView(probes[i].irradiance.view()) //
					.setSampler(GpuAssetManager->GetDefaultSampler())
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);


				// PERF:
				vk::WriteDescriptorSet depthWrite{};
				depthWrite
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler) //
					.setImageInfo(viewInfo)
					.setDstBinding(0u)
					.setDstSet(gridDescSet)
					.setDstArrayElement(i); // PERF:
				Device->updateDescriptorSets({ depthWrite }, nullptr);
			}
		}
	}


	shouldBuild.Set();
}
