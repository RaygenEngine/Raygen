#include "pch.h"
#include "GpuEnvironmentMap.h"

#include "assets/pods/Cubemap.h"
#include "assets/pods/EnvironmentMap.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/wrappers/RBuffer.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanUtl.h"


using namespace vl;

GpuEnvironmentMap::GpuEnvironmentMap(PodHandle<EnvironmentMap> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

void GpuEnvironmentMap::Update(const AssetUpdateInfo&)
{
	auto envmapPod = podHandle.Lock();
	ClearDependencies();
	AddDependencies(envmapPod->skybox, envmapPod->irradiance, envmapPod->prefiltered, envmapPod->brdfLut);

	skybox = GpuAssetManager->GetGpuHandle(envmapPod->skybox);

	skybox.Lock().cubemap->GenerateMipmapsAndTransitionEach(
		vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	irradiance = GpuAssetManager->GetGpuHandle(envmapPod->irradiance);
	prefiltered = GpuAssetManager->GetGpuHandle(envmapPod->prefiltered);
	brdfLut = GpuAssetManager->GetGpuHandle(envmapPod->brdfLut);

	descriptorSet = Layouts->envmapLayout.GetDescriptorSet();

	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	std::array cubemaps = {
		skybox,
		irradiance,
		prefiltered,
	};

	// PERF:
	std::array<vk::WriteDescriptorSet, 4> descriptorWrites;
	std::array<vk::DescriptorImageInfo, 4> imageInfos;

	for (uint32 i = 0u; i < 3u; ++i) {
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

	// sampler
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(vk::Filter::eLinear) //
		.setMinFilter(vk::Filter::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setMipLodBias(0.f) // CHECK:
		.setMinLod(0.f)
		.setMaxLod(32.f); // CHECK:


	brdfSampler = vl::Device->createSamplerUnique(samplerInfo);

	imageInfos[3]
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(brdfLut.Lock().image->GetView())
		.setSampler(brdfSampler.get());

	descriptorWrites[3]
		.setDstSet(descriptorSet) //
		.setDstBinding(3u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfos[3u])
		.setPTexelBufferView(nullptr);

	// single call to update all descriptor sets with the new depth image
	vl::Device->updateDescriptorSets(descriptorWrites, {});
}
