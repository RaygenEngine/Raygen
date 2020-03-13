#include "pch.h"
#include "renderer/asset/Material.h"

#include "asset/AssetManager.h"
#include "asset/pods/MaterialPod.h"
#include "renderer/asset/GpuAssetManager.h"
#include "renderer/VulkanLayer.h"
#include "renderer/wrapper/Device.h"

GpuAssetBaseTyped<MaterialPod>::GpuAssetBaseTyped(PodHandle<MaterialPod> podHandle)
{
	auto data = podHandle.Lock();

	matData.baseColorFactor = data->baseColorFactor;
	matData.emissiveFactor = glm::vec4(data->emissiveFactor, 1.0);
	matData.metallicFactor = data->metallicFactor;
	matData.roughnessFactor = data->roughnessFactor;
	matData.normalScale = data->normalScale;
	matData.occlusionStrength = data->occlusionStrength;

	// text coord indices
	matData.baseColorTexcoordIndex = data->baseColorTexCoordIndex;
	matData.metallicRoughnessTexcoordIndex = data->metallicRoughnessTexCoordIndex;
	matData.emissiveTexcoordIndex = data->emissiveTexCoordIndex;
	matData.normalTexcoordIndex = data->normalTexCoordIndex;
	matData.occlusionTexcoordIndex = data->occlusionTexCoordIndex;

	// alpha mask
	matData.alphaCutoff = data->alphaCutoff;
	matData.mask = data->alphaMode == MaterialPod::AlphaMode::MASK;

	baseColorTexture = GpuAssetManager.GetGpuHandle(data->baseColorTexture);
	metallicRoughnessTexture = GpuAssetManager.GetGpuHandle(data->metallicRoughnessTexture);
	occlusionTexture = GpuAssetManager.GetGpuHandle(data->occlusionTexture);
	normalTexture = GpuAssetManager.GetGpuHandle(data->normalTexture);
	emissiveTexture = GpuAssetManager.GetGpuHandle(data->emissiveTexture);

	// CHECK: upload once for now (not dynamic changes)
	materialUBO.reset(new Buffer(sizeof(UBO_Material), vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));

	materialUBO->UploadData(&matData, sizeof(UBO_Material));


	// descriptors
	descriptorSet = Layer->geomPass.GetMaterialDescriptorSet();

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

	auto UpdateImageSamplerInDescriptorSet = [&](GpuHandle<TexturePod> texture, uint32 dstBinding) {
		auto& text = GpuAssetManager.LockHandle(texture);

		vk::DescriptorImageInfo imageInfo{};
		imageInfo
			.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
			.setImageView(text.view.get())
			.setSampler(text.sampler.get());

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

	UpdateImageSamplerInDescriptorSet(baseColorTexture, 1u);
	UpdateImageSamplerInDescriptorSet(metallicRoughnessTexture, 2u);
	UpdateImageSamplerInDescriptorSet(occlusionTexture, 3u);
	UpdateImageSamplerInDescriptorSet(normalTexture, 4u);
	UpdateImageSamplerInDescriptorSet(emissiveTexture, 5u);
}
