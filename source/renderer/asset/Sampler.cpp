#include "pch.h"
#include "renderer/asset/Sampler.h"

#include "asset/AssetManager.h"
#include "renderer/VulkanUtl.h"
#include "renderer/VulkanLayer.h"
#include "renderer/wrapper/Buffer.h"
#include "renderer/wrapper/Device.h"


GpuAssetBaseTyped<SamplerPod>::GpuAssetBaseTyped(PodHandle<SamplerPod> podHandle)
{
	auto textureData = podHandle.Lock();

	// sampler
	// NEXT: values should be chosen based on Texture pod
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(GetFilter(textureData->magFilter)) //
		.setMinFilter(GetFilter(textureData->minFilter))
		.setAddressModeU(GetWrapping(textureData->wrapU))
		.setAddressModeV(GetWrapping(textureData->wrapV))
		.setAddressModeW(GetWrapping(textureData->wrapW))
		// PERF:
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)

		// CHECK: texture pod should match the vk sampler
		.setMipmapMode(GetMipmapFilter(textureData->magFilter))
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(0.f);

	sampler = Device->createSamplerUnique(samplerInfo);
}
