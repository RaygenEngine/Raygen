#include "GpuCubemap.h"

#include "assets/AssetRegistry.h"
#include "assets/pods/Cubemap.h"
#include "rendering/wrappers/Buffer.h"
#include "rendering/VkCoreIncludes.h"

using namespace vl;

GpuCubemap::GpuCubemap(PodHandle<Cubemap> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

void GpuCubemap::Update(const AssetUpdateInfo&)
{
	auto cubemapPod = podHandle.Lock();
	ClearDependencies();

	vk::Format format = rvk::getFormat(cubemapPod->format);

	cubemap = RCubemap(fmt::format("Cubemap: {}", AssetRegistry::GetPodUri(podHandle)), cubemapPod->resolution, format,
		vk::ImageLayout::eTransferDstOptimal, cubemapPod->mipCount,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled);

	vk::DeviceSize bufferSize = cubemapPod->data.size();

	RBuffer stagingbuffer{ bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingbuffer.UploadData(cubemapPod->data);

	cubemap.CopyBuffer(stagingbuffer, cubemapPod->format == ImageFormat::Hdr ? 16llu : 4llu, cubemapPod->mipCount);

	cubemap.BlockingTransitionToLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}
