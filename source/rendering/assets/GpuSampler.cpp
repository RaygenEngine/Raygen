#include "pch.h"
#include "GpuSampler.h"

#include "rendering/VulkanUtl.h"
#include "rendering/renderer/Renderer.h"
#include "rendering/objects/Buffer.h"
#include "rendering/Device.h"

using namespace vl;

Sampler::Gpu::Gpu(PodHandle<Sampler> podHandle)
{
	auto textureData = podHandle.Lock();

	// sampler
	// NEXT: values should be chosen based on Texture pod
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(vl::GetTextureFilter(textureData->magFilter)) //
		.setMinFilter(vl::GetTextureFilter(textureData->minFilter))
		.setAddressModeU(vl::GetWrapping(textureData->wrapU))
		.setAddressModeV(vl::GetWrapping(textureData->wrapV))
		.setAddressModeW(vl::GetWrapping(textureData->wrapW))

		// PERF:
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)

		// CHECK: texture pod should match the vk sampler
		.setMipmapMode(vl::GetMipmapFilter(textureData->mipmapFilter))
		.setMipLodBias(0.f) // CHECK:
		.setMinLod(0.f)
		.setMaxLod(32.f); // CHECK:

	sampler = vl::Device->createSamplerUnique(samplerInfo);
}
