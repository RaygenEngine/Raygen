#include "pch.h"
#include "renderer/asset/Texture.h"

#include "asset/AssetManager.h"
#include "renderer/wrapper/Device.h"
#include "renderer/wrapper/Buffer.h"

Texture::Texture(PodHandle<TexturePod> podHandle)
{
	auto textureData = podHandle.Lock();
	auto imgData = textureData->image.Lock();

	vk::DeviceSize imageSize = imgData->data.size();

	Buffer stagingBuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingBuffer.UploadData(imgData->data.data(), static_cast<size_t>(imageSize));

	vk::Format format = imgData->isHdr ? vk::Format::eR32G32B32A32Sfloat : vk::Format::eR8G8B8A8Srgb;

	image = std::make_unique<Image>(imgData->width, imgData->height, format, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// copy (internally transitions to transfer optimal)
	image->CopyBufferToImage(stagingBuffer);


	// finally transiton to graphics layout for shader access
	image->TransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	// request
	view = image->RequestImageView2D_0_0();

	// sampler
	// NEXT: values should be chosen based on Texture pod
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(vk::Filter::eLinear) //
		.setMinFilter(vk::Filter::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
		// PERF:
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(0.f);

	sampler = Device->createSamplerUnique(samplerInfo);
}
