#include "pch.h"

#include "assets/model/Material.h"
#include "assets/model/GltfAux.h"

namespace Assets
{
	Material::Material(DiskAsset* pAsset, const std::string& name)
		: DiskAssetPart(pAsset, name),
	      m_baseColorTextureSampler(this, "baseColorSampler"),
	      m_metallicRoughnessTextureSampler(this, "metallicRoughnessSampler"),
		  m_normalTextureSampler(this, "normalSampler"),
		  m_occlusionTextureSampler(this, "occlusionSampler"),
		  m_emissiveTextureSampler(this, "emissiveSampler"),
		  m_baseColorFactor(1.f, 1.f, 1.f, 1.f),
		  m_emissiveFactor(0.f, 0.f, 0.f),
		  m_metallicFactor(1.f),
		  m_roughnessFactor(1.f),
		  m_normalScale(1.f),
		  m_occlusionStrength(1.f),
		  m_alphaMode(AM_OPAQUE),
		  m_alphaCutoff(0.5f),
		  m_doubleSided(false)
	{
	}

	void Material::LoadFromGltfData(const tinygltf::Model& modelData, const tinygltf::Material& materialData)
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
		m_baseColorTextureSampler.LoadFromGltfData(modelData, baseColorTextureInfo.index, baseColorTextureInfo.texCoord);
		
		auto& metallicRougnessTextureInfo = materialData.pbrMetallicRoughness.metallicRoughnessTexture;
		m_metallicRoughnessTextureSampler.LoadFromGltfData(modelData, metallicRougnessTextureInfo.index, metallicRougnessTextureInfo.texCoord);
		
		auto& emissiveTextureInfo = materialData.emissiveTexture;
		m_emissiveTextureSampler.LoadFromGltfData(modelData, emissiveTextureInfo.index, emissiveTextureInfo.texCoord);
		
		auto& normalTextureInfo = materialData.normalTexture;
		m_normalTextureSampler.LoadFromGltfData(modelData, normalTextureInfo.index, normalTextureInfo.texCoord, false);
		
		auto& occlusionTextureInfo = materialData.occlusionTexture;
		m_occlusionTextureSampler.LoadFromGltfData(modelData, occlusionTextureInfo.index, occlusionTextureInfo.texCoord, false);
	}
};

