#include "pch.h"
#include "renderer/Texture.h"

#include "asset/AssetManager.h"
#include "renderer/LogicalDevice.h"
#include "renderer/VulkanLayer.h"

Texture::Texture(PodHandle<TexturePod> podHandle)
{
	auto& device = VulkanLayer::device;


	auto textureData = podHandle.Lock();
	auto imgData = textureData->image.Lock();

	vk::DeviceSize imageSize = imgData->data.size();

	vk::UniqueBuffer stagingBuffer;
	vk::UniqueDeviceMemory stagingBufferMemory;
	device->CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
		stagingBufferMemory);

	// copy data to buffer
	void* bufferData = device->mapMemory(stagingBufferMemory.get(), 0, imageSize);
	memcpy(bufferData, imgData->data.data(), static_cast<size_t>(imageSize));
	device->unmapMemory(stagingBufferMemory.get());


	vk::Format format = imgData->isHdr ? vk::Format::eR32G32B32A32Sfloat : vk::Format::eR8G8B8A8Srgb;

	image = std::make_unique<Image>(imgData->width, imgData->height, format, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// copy (internally transitions to transfer optimal)
	image->CopyBufferToImage(stagingBuffer.get());


	// finally transiton to graphics layout for shader access
	image->TransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	// request
	view = image->RequestImageView2D_0_0();

	// sampler
	// NEXT: values should be chosen based on Texture pod
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.setMagFilter(vk::Filter::eLinear)
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

	sampler = device->createSamplerUnique(samplerInfo);
}
