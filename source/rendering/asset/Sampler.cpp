#include "pch.h"
#include "rendering/asset/Sampler.h"

#include "assets/AssetManager.h"
#include "rendering/VulkanUtl.h"
#include "rendering/renderer/Renderer.h"
#include "rendering/wrapper/Buffer.h"
#include "rendering/Device.h"


GpuAssetBaseTyped<SamplerPod>::GpuAssetBaseTyped(PodHandle<SamplerPod> podHandle)
{
	auto textureData = podHandle.Lock();

	// sampler
	// NEXT: values should be chosen based on Texture pod
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(GetTextureFilter(textureData->magFilter)) //
		.setMinFilter(GetTextureFilter(textureData->minFilter))
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
		.setMipmapMode(GetMipmapFilter(textureData->mipmapFilter))
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(0.f);

	sampler = Device->createSamplerUnique(samplerInfo);
}
