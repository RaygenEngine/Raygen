#include "pch.h"
#include "SceneSpotlight.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"

SceneSpotlight::SceneSpotlight()
	: SceneStruct<Spotlight_Ubo>()
{
	for (uint32 i = 0; i < 3; ++i) {
		descSets[i] = vl::Layouts->spotlightDescLayout.GetDescriptorSet();
	}
}

void SceneSpotlight::UpdateShadowmap(uint32 width, uint32 height)
{
	shadowmap = std::make_unique<vl::RDepthmap>(vl::Renderer->GetShadowmapRenderPass(), width, height);

	auto quadSampler = vl::GpuAssetManager->GetDefaultSampler();

	// one for each descSet
	std::array<vk::WriteDescriptorSet, 3> descriptorWrites;
	for (uint32 i = 0; i < 3; ++i) {
		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(shadowmap->GetDepthAttachment()->GetView())
			.setSampler(quadSampler);

		descriptorWrites[i]
			.setDstSet(descSets[i]) //
			.setDstBinding(1u)      // 0 is for the Ubo
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setPBufferInfo(nullptr)
			.setPImageInfo(&imageInfo)
			.setPTexelBufferView(nullptr);
	}

	// single call to update all descriptor sets with the new depth image
	vl::Device->updateDescriptorSets(descriptorWrites, {});
}
