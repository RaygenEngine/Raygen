#include "pch.h"
#include "GpuEnvironmentMap.h"

#include "assets/pods/Cubemap.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/objects/Buffer.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanUtl.h"

using namespace vl;

EnvironmentMap::Gpu::Gpu(PodHandle<EnvironmentMap> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

void EnvironmentMap::Gpu::Update(const AssetUpdateInfo&)
{
	auto envmapPod = podHandle.Lock();
	ClearDependencies();
	AddDependencies(envmapPod->skybox, envmapPod->irradiance, envmapPod->prefiltered, envmapPod->brdfLut);

	skybox = GpuAssetManager->GetGpuHandle(envmapPod->skybox);
	irradiance = GpuAssetManager->GetGpuHandle(envmapPod->irradiance);
	prefiltered = GpuAssetManager->GetGpuHandle(envmapPod->prefiltered);
	brdfLut = GpuAssetManager->GetGpuHandle(envmapPod->brdfLut);

	descriptorSet = Layouts->envmapLayout.GetDescriptorSet();

	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	std::array cubemaps = {
		skybox,
		irradiance,
		prefiltered,
		brdfLut,
	};

	// PERF:
	std::array<vk::WriteDescriptorSet, 4> descriptorWrites;
	std::array<vk::DescriptorImageInfo, 4> imageInfos;

	for (uint32 i = 0u; i < 4u; ++i) {
		auto& cubemapPod = cubemaps[i].Lock();

		imageInfos[i]
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(cubemapPod.cubemap->GetView())
			.setSampler(quadSampler);

		descriptorWrites[i]
			.setDstSet(descriptorSet) //
			.setDstBinding(i)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
			.setDescriptorCount(1u)
			.setPBufferInfo(nullptr)
			.setPImageInfo(&imageInfos[i])
			.setPTexelBufferView(nullptr);
	}

	// single call to update all descriptor sets with the new depth image
	vl::Device->updateDescriptorSets(descriptorWrites, {});
}
