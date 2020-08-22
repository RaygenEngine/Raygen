#include "pch.h"
#include "GpuCubemap.h"

#include "assets/pods/Cubemap.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
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
		vk::MemoryPropertyFlagBits::eDeviceLocal);

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

	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(cubemap.view())
		.setSampler(quadSampler);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(descriptorSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setImageInfo(imageInfo);

	// single call to update all descriptor sets with the new depth image
	Device->updateDescriptorSets({ descriptorWrite }, {});
}
