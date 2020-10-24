#include "GpuCubemap.h"

#include "assets/AssetRegistry.h"
#include "assets/pods/Cubemap.h"
#include "rendering/Layouts.h"
#include "rendering/util/WriteDescriptorSets.h"
#include "rendering/wrappers/Buffer.h"

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

	cubemap = RCubemap(cubemapPod->resolution, cubemapPod->mipCount, format, //
		vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, fmt::format("Cubemap: {}", AssetRegistry::GetPodUri(podHandle)));

	// transiton all mips to transfer optimal
	cubemap.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	vk::DeviceSize bufferSize = cubemapPod->data.size();

	RBuffer stagingbuffer{ bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingbuffer.UploadData(cubemapPod->data);

	cubemap.CopyBuffer(stagingbuffer, cubemapPod->format == ImageFormat::Hdr ? 16llu : 4llu, cubemapPod->mipCount);

	cubemap.BlockingTransitionToLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	descriptorSet = Layouts->cubemapLayout.AllocDescriptorSet();

	rvk::writeDescriptorImages(descriptorSet, 0u, { cubemap.view() });
}
