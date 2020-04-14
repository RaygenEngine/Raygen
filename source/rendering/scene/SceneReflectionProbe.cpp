#include "pch.h"
#include "SceneReflectionProbe.h"

#include "assets/pods/Cubemap.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"

SceneReflectionProbe::SceneReflectionProbe()
	: SceneStruct<Ambient_Ubo>()
{
	for (uint32 i = 0; i < 3; ++i) {
		descSets[i] = vl::Layouts->ambientDescLayout.GetDescriptorSet();
	}
}

void SceneReflectionProbe::UploadCubemap(PodHandle<Cubemap> cubemapData)
{
	cubemap = vl::GpuAssetManager->GetGpuHandle(cubemapData);

	auto& cubemapAsset = cubemap.Lock();

	auto quadSampler = vl::GpuAssetManager->GetDefaultSampler();

	// one for each descSet
	std::array<vk::WriteDescriptorSet, 3> descriptorWrites;
	for (uint32 i = 0; i < 3; ++i) {
		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(cubemapAsset.cubemap->GetView())
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
