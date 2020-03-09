#include "pch.h"
#include "renderer/asset/Material.h"

#include "asset/AssetManager.h"
#include "asset/pods/MaterialPod.h"

Material::Material(PodHandle<MaterialPod> podHandle)
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

	baseColorTexture = std::make_unique<Texture>(data->baseColorTexture);
	metallicRoughnessTexture = std::make_unique<Texture>(data->metallicRoughnessTexture);
	occlusionTexture = std::make_unique<Texture>(data->occlusionTexture);
	normalTexture = std::make_unique<Texture>(data->normalTexture);
	emissiveTexture = std::make_unique<Texture>(data->emissiveTexture);

	// CHECK: upload once for now (not dynamic changes)
	materialUBO = std::make_unique<Buffer>(sizeof(UBO_Material), vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	materialUBO->UploadData(&matData, sizeof(UBO_Material));
}
