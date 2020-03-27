#include "pch.h"
#include "Image.h"

#include "rendering/Device.h"
#include "rendering/renderer/Renderer.h"
#include "rendering/VulkanUtl.h"
#include "rendering/wrapper/Buffer.h"


GpuAssetBaseTyped<ImagePod>::GpuAssetBaseTyped(PodHandle<ImagePod> podHandle)
{
	auto imgData = podHandle.Lock();

	vk::DeviceSize imageSize = imgData->data.size();

	Buffer stagingBuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingBuffer.UploadData(imgData->data.data(), static_cast<size_t>(imageSize));

	vk::Format format = imgData->isHdr ? vk::Format::eR32G32B32A32Sfloat : vk::Format::eR8G8B8A8Srgb;

	image.reset(new ImageObj(imgData->width, imgData->height, format, vk::ImageTiling::eOptimal,
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
}
