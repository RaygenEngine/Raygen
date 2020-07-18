#include "pch.h"
#include "GpuCubemap.h"

#include "assets/pods/Cubemap.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanUtl.h"
#include "rendering/wrappers/RBuffer.h"

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

	vk::Format format = GetFormat(cubemapPod->format);

	cubemap = std::make_unique<vl::RCubemap>(cubemapPod->resolution, cubemapPod->mipCount, format, //
		vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// transiton all mips to transfer optimal
	cubemap->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);

	vk::DeviceSize bufferSize = cubemapPod->data.size();

	vl::RBuffer stagingbuffer{ bufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	stagingbuffer.UploadData(cubemapPod->data);

	cubemap->CopyBuffer(stagingbuffer, cubemapPod->format == ImageFormat::Hdr ? 16llu : 4llu, cubemapPod->mipCount);

	cubemap->BlockingTransitionToLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	descriptorSet = Layouts->cubemapLayout.GetDescriptorSet();

	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(cubemap->GetView())
		.setSampler(quadSampler);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(descriptorSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo)
		.setPTexelBufferView(nullptr);

	// single call to update all descriptor sets with the new depth image
	vl::Device->updateDescriptorSets({ descriptorWrite }, {});
}
