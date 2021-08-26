#include "GpuSampler.h"

#include "assets/pods/Sampler.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/VkCoreIncludes.h"

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
		.setMagFilter(rvk::getTextureFilter(textureData->magFilter)) //
		.setMinFilter(rvk::getTextureFilter(textureData->minFilter))
		.setAddressModeU(rvk::getWrapping(textureData->wrapU))
		.setAddressModeV(rvk::getWrapping(textureData->wrapV))
		.setAddressModeW(rvk::getWrapping(textureData->wrapW))
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(rvk::getMipmapFilter(textureData->mipmapFilter))
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(32.f);


	sampler = GpuResources::AcquireSampler(samplerInfo);
}
