#include "pch.h"
#include "GpuMaterial.h"

#include "assets/pods/Material.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/renderer/Renderer.h"
#include "rendering/Device.h"

using namespace vl;

Material::Gpu::Gpu(PodHandle<Material> podHandle)
{
	auto data = podHandle.Lock();

	matData.baseColorFactor = data->baseColorFactor;
	matData.emissiveFactor = glm::vec4(data->emissiveFactor, 1.0);
	matData.metallicFactor = data->metallicFactor;
	matData.roughnessFactor = data->roughnessFactor;
	matData.normalScale = data->normalScale;
	matData.occlusionStrength = data->occlusionStrength;

	// alpha mask
	matData.alphaCutoff = data->alphaCutoff;
	matData.mask = data->alphaMode == Material::AlphaMode::Mask;

	baseColorSampler = GpuAssetManager->GetGpuHandle(data->baseColorSampler);
	baseColorImage = GpuAssetManager->GetGpuHandle(data->baseColorImage);

	metallicRoughnessSampler = GpuAssetManager->GetGpuHandle(data->metallicRoughnessSampler);
	metallicRoughnessImage = GpuAssetManager->GetGpuHandle(data->metallicRoughnessImage);

	occlusionSampler = GpuAssetManager->GetGpuHandle(data->occlusionSampler);
	occlusionImage = GpuAssetManager->GetGpuHandle(data->occlusionImage);

	normalSampler = GpuAssetManager->GetGpuHandle(data->normalSampler);
	normalImage = GpuAssetManager->GetGpuHandle(data->normalImage);

	emissiveSampler = GpuAssetManager->GetGpuHandle(data->emissiveSampler);
	emissiveImage = GpuAssetManager->GetGpuHandle(data->emissiveImage);

	// CHECK: upload once for now (not dynamic changes)
	materialUBO = std::make_unique<Buffer<UBO_Material>>(vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	materialUBO->UploadData(matData);

	// descriptors (WIP:
	descriptorSet = Renderer->GetGeometryPass()->GetRegularMaterialDescriptorSet();

	// material uniform sets CHECK: (those buffers should be set again when material changes)
	vk::DescriptorBufferInfo bufferInfo{};

	bufferInfo
		.setBuffer(*materialUBO) //
		.setOffset(0u)
		.setRange(sizeof(UBO_Material));
	vk::WriteDescriptorSet descriptorWrite{};

	descriptorWrite
		.setDstSet(descriptorSet) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1u)
		.setPBufferInfo(&bufferInfo)
		.setPImageInfo(nullptr)
		.setPTexelBufferView(nullptr);

	Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);


	// images (material)

	auto UpdateImageSamplerInDescriptorSet
		= [&](GpuHandle<Sampler> sampler, GpuHandle<::Image> image, uint32 dstBinding) {
			  auto& sam = sampler.Lock();
			  auto& img = image.Lock();

			  vk::DescriptorImageInfo imageInfo{};
			  imageInfo
				  .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				  .setImageView(img.image->GetView())
				  .setSampler(sam.sampler.get());

			  vk::WriteDescriptorSet descriptorWrite{};
			  descriptorWrite
				  .setDstSet(descriptorSet) //
				  .setDstBinding(dstBinding)
				  .setDstArrayElement(0u)
				  .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				  .setDescriptorCount(1u)
				  .setPBufferInfo(nullptr)
				  .setPImageInfo(&imageInfo)
				  .setPTexelBufferView(nullptr);

			  Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
		  };

	UpdateImageSamplerInDescriptorSet(baseColorSampler, baseColorImage, 1u);
	UpdateImageSamplerInDescriptorSet(metallicRoughnessSampler, metallicRoughnessImage, 2u);
	UpdateImageSamplerInDescriptorSet(occlusionSampler, occlusionImage, 3u);
	UpdateImageSamplerInDescriptorSet(normalSampler, normalImage, 4u);
	UpdateImageSamplerInDescriptorSet(emissiveSampler, emissiveImage, 5u);
}
