#include "pch.h"

#include "assets/model/Material.h"
#include "assets/model/GltfAux.h"

namespace Assets
{
	Material::Material(DiskAsset* pAsset, const std::string& name)
		: DiskAssetPart(pAsset, name),
	      m_baseColorTextureSampler(this, "baseColorSampler"),
	      m_occlusionMetallicRoughnessTextureSampler(this, "occlusionMetallicRoughnessSampler"),
		  m_normalTextureSampler(this, "normalSampler", false),
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

		auto& emissiveTextureInfo = materialData.emissiveTexture;
		m_emissiveTextureSampler.Load(modelData, emissiveTextureInfo.index, emissiveTextureInfo.texCoord);

		auto& normalTextureInfo = materialData.normalTexture;
		m_normalTextureSampler.Load(modelData, normalTextureInfo.index, normalTextureInfo.texCoord);

		// handle occlusion metallic roughness packing
		auto& metallicRougnessTextureInfo = materialData.pbrMetallicRoughness.metallicRoughnessTexture;
		auto& occlusionTextureInfo = materialData.occlusionTexture;

		// packed together in a single image
		if (metallicRougnessTextureInfo.index == occlusionTextureInfo.index)
		{
			m_occlusionMetallicRoughnessTextureSampler.Load(modelData, metallicRougnessTextureInfo.index, metallicRougnessTextureInfo.texCoord);
		}
		// otherwise we need to pack them manually
		else
		{
			// pack occlusion texture in red channel
			// pack metallic and roughness to blue and green
			m_occlusionMetallicRoughnessTextureSampler.LoadPacked(modelData, 
				occlusionTextureInfo.index, TC_RED, occlusionTextureInfo.texCoord,
			    metallicRougnessTextureInfo.index, static_cast<TextureChannel>(TC_BLUE | TC_GREEN), metallicRougnessTextureInfo.texCoord);
		}
	}
};

