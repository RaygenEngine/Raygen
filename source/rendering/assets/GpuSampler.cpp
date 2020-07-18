#include "pch.h"
#include "GpuSampler.h"

#include "rendering/VulkanUtl.h"
#include "rendering/Renderer.h"
#include "rendering/wrappers/RBuffer.h"
#include "rendering/Device.h"

using namespace vl;

GpuSampler::GpuSampler(PodHandle<Sampler> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}


void GpuSampler::Update(const AssetUpdateInfo& info)
{
	auto textureData = podHandle.Lock();

	// sampler
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(vl::GetTextureFilter(textureData->magFilter)) //
		.setMinFilter(vl::GetTextureFilter(textureData->minFilter))
		.setAddressModeU(vl::GetWrapping(textureData->wrapU))
		.setAddressModeV(vl::GetWrapping(textureData->wrapV))
		.setAddressModeW(vl::GetWrapping(textureData->wrapW))
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vl::GetMipmapFilter(textureData->mipmapFilter))
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(32.f);


	sampler = vl::Device->createSamplerUnique(samplerInfo);
}
