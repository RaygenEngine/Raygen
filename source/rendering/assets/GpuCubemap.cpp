#include "pch.h"
#include "GpuCubemap.h"

#include "rendering/Device.h"
#include "rendering/objects/Buffer.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanUtl.h"


Cubemap::Gpu::Gpu(PodHandle<Cubemap> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

void Cubemap::Gpu::Update(const AssetUpdateInfo&)
{
	auto cubemapData = podHandle.Lock();

	vk::Format format = vl::GetFormat(cubemapData->format);

	cubemap = std::make_unique<vl::Cubemap>(cubemapData->width, format, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// transiton all mips to transfer optimal
	cubemap->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	for (uint32 i = 0; i < 6u; ++i) {
		vk::DeviceSize imageSize = cubemapData->faces[i].Lock()->data.size();

		vl::RawBuffer stagingBuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		// copy data to buffer
		stagingBuffer.UploadData(cubemapData->faces[i].Lock()->data.data(), static_cast<size_t>(imageSize));


		cubemap->CopyBufferToFace(stagingBuffer, i);
	}

	cubemap->BlockingTransitionToLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	// TEMP______________________________________

	irradiance = std::make_unique<vl::Cubemap>(512, format, vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// transiton all mips to transfer optimal
	irradiance->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	for (uint32 i = 0; i < 6u; ++i) {
		vk::DeviceSize imageSize = cubemapData->irradiance[i].Lock()->data.size();

		vl::RawBuffer stagingBuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		// copy data to buffer
		stagingBuffer.UploadData(cubemapData->irradiance[i].Lock()->data.data(), static_cast<size_t>(imageSize));


		irradiance->CopyBufferToFace(stagingBuffer, i);
	}

	irradiance->BlockingTransitionToLayout(
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}
