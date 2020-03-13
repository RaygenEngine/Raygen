#include "pch.h"
#include "renderer/asset/Texture.h"

#include "asset/AssetManager.h"
#include "renderer/VulkanUtl.h"
#include "renderer/wrapper/Buffer.h"
#include "renderer/wrapper/Device.h"

GpuAssetBaseTyped<TexturePod>::GpuAssetBaseTyped(PodHandle<TexturePod> podHandle)
{
	auto textureData = podHandle.Lock();
	auto imgData = textureData->image.Lock();

	vk::DeviceSize imageSize = imgData->data.size();

	Buffer stagingBuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingBuffer.UploadData(imgData->data.data(), static_cast<size_t>(imageSize));

	vk::Format format = imgData->isHdr ? vk::Format::eR32G32B32A32Sfloat : vk::Format::eR8G8B8A8Srgb;

	image.reset(new Image(imgData->width, imgData->height, format, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal));

	// transiton to transfer optimal
	image->TransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	// copy (internally transitions to transfer optimal)
	image->CopyBufferToImage(stagingBuffer);

	// finally transiton to graphics layout for shader access
	image->TransitionToLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	// request
	view = image->RequestImageView2D_0_0();

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
