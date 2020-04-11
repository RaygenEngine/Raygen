#include "pch.h"
#include "GpuImage.h"

#include "rendering/Device.h"
#include "rendering/objects/Buffer.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanUtl.h"

using namespace vl;

::Image::Gpu::Gpu(PodHandle<::Image> podHandle)
{
	auto imgData = podHandle.Lock();

	vk::DeviceSize imageSize = imgData->data.size();

	RawBuffer stagingBuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingBuffer.UploadData(imgData->data.data(), static_cast<size_t>(imageSize));

	vk::Format format;
	switch (imgData->format) {
		case ImageFormat::Hdr: format = vk::Format::eR32G32B32A32Sfloat; break;
		case ImageFormat::Srgb: format = vk::Format::eR8G8B8A8Srgb; break;
		case ImageFormat::Unorm: format = vk::Format::eR8G8B8A8Unorm; break;
	}

	uint32 mipLevels = static_cast<uint32>(std::floor(std::log2(glm::max(imgData->width, imgData->height)))) + 1;

	image = std::make_unique<Image2D>(imgData->width, imgData->height, mipLevels, format, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// transiton all mips to transfer optimal
	image->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	// copy (internally transitions to transfer optimal)
	image->CopyBufferToImage(stagingBuffer);

	image->GenerateMipmapsAndTransitionEach(
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}
