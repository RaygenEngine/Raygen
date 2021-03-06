#include "GpuImage.h"

#include "assets/AssetRegistry.h"
#include "assets/pods/Image.h"
#include "rendering/Renderer.h"

using namespace vl;

GpuImage::GpuImage(PodHandle<::Image> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

// PERF: Check usage, probably wrong because we remake "image" member variable.
void GpuImage::Update(const AssetUpdateInfo& info)
{
	auto imgData = podHandle.Lock();

	vk::DeviceSize imageSize = imgData->data.size();

	RBuffer stagingbuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingbuffer.UploadData(imgData->data);

	vk::Format format = rvk::getFormat(imgData->format);

	uint32 mipLevels = static_cast<uint32>(std::floor(std::log2(glm::max(imgData->width, imgData->height)))) + 1;

	image = RImage2D(AssetRegistry::GetPodUri(podHandle),
		vk::Extent2D{ static_cast<uint32>(imgData->width), static_cast<uint32>(imgData->height) }, format,
		vk::ImageLayout::eTransferDstOptimal, mipLevels,
		vk::ImageUsageFlagBits::eTransferSrc | vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

	// copy (internally transitions to transfer optimal)
	image.CopyBufferToImage(stagingbuffer);

	image.GenerateMipmapsAndTransitionEach(
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}
