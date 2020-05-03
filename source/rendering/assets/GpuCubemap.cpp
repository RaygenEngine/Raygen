#include "pch.h"
#include "GpuCubemap.h"

#include "rendering/Device.h"
#include "rendering/objects/Buffer.h"
#include "rendering/Renderer.h"
#include "rendering/VulkanUtl.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Layouts.h"
#include "rendering/passes/IrradianceMapCalculation.h"

Cubemap::Gpu::Gpu(PodHandle<Cubemap> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}

void Cubemap::Gpu::Update(const AssetUpdateInfo&)
{
	auto cubemapData = podHandle.Lock();
	ClearDependencies();
	AddDependencies(cubemapData->faces);
	AddDependencies(cubemapData->irradiance);

	vk::Format format = vl::GetFormat(cubemapData->format);

	cubemap = std::make_unique<vl::Cubemap>(cubemapData->resolution, format, vk::ImageTiling::eOptimal,
		vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	// transiton all mips to transfer optimal
	cubemap->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);


	for (uint32 i = 0; i < 6u; ++i) {
		auto face = cubemapData->faces[i].Lock();

		vk::DeviceSize imageSize = face->data.size();

		vl::RawBuffer stagingBuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		// copy data to buffer
		stagingBuffer.UploadData(face->data);


		cubemap->CopyBufferToFace(stagingBuffer, i);
	}

	cubemap->BlockingTransitionToLayout(vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);

	// TEMP______________________________________ has hacks to work

	irradiance
		= std::make_unique<vl::Cubemap>(cubemapData->irradiance[0].Lock()->width, format, vk::ImageTiling::eOptimal,
			vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
			vk::MemoryPropertyFlagBits::eDeviceLocal);

	// transiton all mips to transfer optimal
	irradiance->BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);


	for (uint32 i = 0; i < 6u; ++i) {
		auto face = cubemapData->irradiance[i].Lock();

		vk::DeviceSize imageSize = face->data.size();

		vl::RawBuffer stagingBuffer{ imageSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		// copy data to buffer
		stagingBuffer.UploadData(face->data);


		irradiance->CopyBufferToFace(stagingBuffer, i);
	}

	irradiance->BlockingTransitionToLayout(
		vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);


	descriptorSet = vl::Layouts->cubemapLayout.GetDescriptorSet();

	auto quadSampler = vl::GpuAssetManager->GetDefaultSampler();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(cubemap->GetView())
		.setSampler(quadSampler);

	vk::DescriptorImageInfo imageInfo2{};
	imageInfo2
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(irradiance->GetView())
		.setSampler(quadSampler);


	vk::WriteDescriptorSet descriptorWrite;


	descriptorWrite
		.setDstSet(descriptorSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo)
		.setPTexelBufferView(nullptr);

	vk::WriteDescriptorSet descriptorWrite2;
	descriptorWrite2
		.setDstSet(descriptorSet) //
		.setDstBinding(1u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo2)
		.setPTexelBufferView(nullptr);

	std::array info{ descriptorWrite, descriptorWrite2 };

	// single call to update all descriptor sets with the new depth image
	vl::Device->updateDescriptorSets(info, {});
}
