#include "pch.h"
#include "GpuImage.h"

#include "assets/pods/Image.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanUtl.h"
#include "rendering/wrappers/RBuffer.h"

using namespace vl;

GpuImage::GpuImage(PodHandle<::Image> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

// NEXT: Check usage, probably wrong because we remake "image" member variable.
void GpuImage::Update(const AssetUpdateInfo& info)
{
	auto imgData = podHandle.Lock();

	vk::DeviceSize imageSize = imgData->data.size();

	RBuffer stagingbuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingbuffer.UploadData(imgData->data);

	vk::Format format = GetFormat(imgData->format);

	uint32 mipLevels = static_cast<uint32>(std::floor(std::log2(glm::max(imgData->width, imgData->height)))) + 1;

	image = std::make_unique<RImage2D>(imgData->width, imgData->height, mipLevels, format, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// transiton all mips to transfer optimal
	image->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	// copy (internally transitions to transfer optimal)
	image->CopyBufferToImage(stagingbuffer);

	image->GenerateMipmapsAndTransitionEach(
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}
