#include "pch/pch.h"

#include "renderer/LogicalDevice.h"

#include "renderer/VulkanLayer.h"

#include "renderer/Texture.h"
#include "asset/AssetManager.h"


Texture::Texture(PodHandle<TexturePod> podHandle)
{
	auto& device = VulkanLayer::device;


	auto data = podHandle.Lock();

	// WIP: get first image for now
	auto imgData = data->images[0].Lock();

	// if(isHdr) data -> float* else data -> byte*
	vk::DeviceSize imageSize = imgData->height * imgData->width * 4 * (imgData->isHdr ? 4 : 1);


	vk::UniqueBuffer stagingBuffer;
	vk::UniqueDeviceMemory stagingBufferMemory;
	device->CreateBuffer(imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, stagingBuffer,
		stagingBufferMemory);

	// copy data to buffer
	void* bufferData = device->handle->mapMemory(stagingBufferMemory.get(), 0, imageSize);
	memcpy(bufferData, imgData->data.data(), static_cast<size_t>(imageSize));
	device->handle->unmapMemory(stagingBufferMemory.get());


	vk::Format format = imgData->isHdr ? vk::Format::eR32G32B32A32Sfloat : vk::Format::eR8G8B8A8Srgb;

	// WIP: based on texture
	device->CreateImage(imgData->width, imgData->height, format, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, handle, memory);

	// transition from undefined to transfer layout
	device->TransitionImageLayout(
		handle.get(), format, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	// copy
	device->CopyBufferToImage(
		stagingBuffer.get(), handle.get(), static_cast<uint32>(imgData->width), static_cast<uint32>(imgData->height));

	// finally transiton to graphics layout for shader access
	device->TransitionImageLayout(
		handle.get(), format, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);


	vk::ImageViewCreateInfo viewInfo{};
	viewInfo.setImage(handle.get()).setViewType(vk::ImageViewType::e2D).setFormat(format);
	viewInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0u)
		.setLevelCount(1u)
		.setBaseArrayLayer(0u)
		.setLayerCount(1u);

	view = device->handle->createImageViewUnique(viewInfo);

	// sampler
	// WIP: values should be chosen based on Texture pod
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo.setMagFilter(vk::Filter::eLinear)
		.setMinFilter(vk::Filter::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eRepeat)
		.setAddressModeV(vk::SamplerAddressMode::eRepeat)
		.setAddressModeW(vk::SamplerAddressMode::eRepeat)
		// PERF:
		.setAnisotropyEnable(VK_FALSE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(0.f);

	sampler = device->handle->createSamplerUnique(samplerInfo);
}