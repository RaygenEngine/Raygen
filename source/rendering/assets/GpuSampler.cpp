#include "pch.h"
#include "GpuSampler.h"

#include "assets/pods/Sampler.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanUtl.h"
#include "rendering/resource/GpuResources.h"

using namespace vl;

GpuSampler::GpuSampler(PodHandle<Sampler> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

vl::GpuSampler::~GpuSampler()
{
	GpuResources::ReleaseSampler(sampler);
}

void GpuSampler::Update(const AssetUpdateInfo& info)
{
	const auto textureData = podHandle.Lock();

	// sampler
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(GetTextureFilter(textureData->magFilter)) //
		.setMinFilter(GetTextureFilter(textureData->minFilter))
		.setAddressModeU(GetWrapping(textureData->wrapU))
		.setAddressModeV(GetWrapping(textureData->wrapV))
		.setAddressModeW(GetWrapping(textureData->wrapW))
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(GetMipmapFilter(textureData->mipmapFilter))
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(32.f);


	sampler = GpuResources::AcquireSampler(samplerInfo);
}
