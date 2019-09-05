#include "pch.h"

#include "assets/model/Material.h"
#include "assets/model/GltfAux.h"

void Material::Load(const tinygltf::Model& modelData, const tinygltf::Material& materialData)
{
	// factors
	auto bFactor = materialData.pbrMetallicRoughness.baseColorFactor;
	m_baseColorFactor = { bFactor[0], bFactor[1], bFactor[2], bFactor[3] };
	m_metallicFactor = static_cast<float>(materialData.pbrMetallicRoughness.metallicFactor);
	m_roughnessFactor = static_cast<float>(materialData.pbrMetallicRoughness.roughnessFactor);
	auto eFactor = materialData.emissiveFactor;
	m_emissiveFactor = { eFactor[0], eFactor[1], eFactor[2] };

	// scales/strenghts
	m_normalScale = static_cast<float>(materialData.normalTexture.scale);
	m_occlusionStrength = static_cast<float>(materialData.occlusionTexture.strength);

	// alpha
	m_alphaMode = GetAlphaModeFromGltf(materialData.alphaMode);

	m_alphaCutoff = static_cast<float>(materialData.alphaCutoff);
	// doublesided-ness
	m_doubleSided = materialData.doubleSided;

	// samplers
	auto& baseColorTextureInfo = materialData.pbrMetallicRoughness.baseColorTexture;
	m_baseColorTextureSampler.Load(modelData, baseColorTextureInfo.index, baseColorTextureInfo.texCoord);
	
	auto& metallicRougnessTextureInfo = materialData.pbrMetallicRoughness.metallicRoughnessTexture;
	m_metallicRoughnessTextureSampler.Load(modelData, metallicRougnessTextureInfo.index, metallicRougnessTextureInfo.texCoord);
	
	auto& emissiveTextureInfo = materialData.emissiveTexture;
	m_emissiveTextureSampler.Load(modelData, emissiveTextureInfo.index, emissiveTextureInfo.texCoord);

	auto& occlusionTextureInfo = materialData.occlusionTexture;
	m_occlusionTextureSampler.Load(modelData, occlusionTextureInfo.index, occlusionTextureInfo.texCoord);
	
	auto& normalTextureInfo = materialData.normalTexture;
	m_normalTextureSampler.Load(modelData, normalTextureInfo.index, normalTextureInfo.texCoord);
}
