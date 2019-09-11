#include "pch.h"

#include "assets/model/Material.h"
#include "system/Engine.h"
#include "assets/AssetManager.h"
#include "assets/other/gltf/GltfFile.h"
#include "assets/other/gltf/GltfAux.h"

#include "tinygltf/tiny_gltf.h"

bool Material::Load()
{
	// TODO check if sub asset


	// if sub asset
	const auto parentAssetPath = m_uri.parent_path();

	// gltf parent TODO: use a loader
	if (parentAssetPath.extension().compare(".gltf") == 0)
	{
		GltfFile* gltfFile = Engine::GetAssetManager()->MaybeGenerateAsset<GltfFile>(parentAssetPath);
		if (!Engine::GetAssetManager()->Load(gltfFile))
			return false;

		auto gltfData = gltfFile->GetGltfData();
		const auto thisPath = m_uri.filename();
		const auto index = std::stoi(thisPath);

		auto& materialData = gltfData->materials.at(index);

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
		m_alphaMode = GltfAux::GetAlphaMode(materialData.alphaMode);

		m_alphaCutoff = static_cast<float>(materialData.alphaCutoff);
		// doublesided-ness
		m_doubleSided = materialData.doubleSided;


		auto LoadTexture = [&](auto textureInfo, Texture*& texture, int32& textCoordIndex)
		{
			if (textureInfo.index != -1)
			{
				auto& gltfTexture = gltfData->textures.at(textureInfo.index);
				
				fs::path subAssetPath = std::to_string(textureInfo.index) + ".";

				subAssetPath += fs::path("texture_" + gltfTexture.name);

				fs::path subPartPath = parentAssetPath / subAssetPath;

				auto textureAsset = Engine::GetAssetManager()->MaybeGenerateAsset<Texture>(subPartPath);
				if (!Engine::GetAssetManager()->Load(textureAsset))
					return false;

				texture = textureAsset;

				textCoordIndex = textureInfo.texCoord;
			}
			// TODO: else default
			return true;
		};
		
		// samplers
		auto& baseColorTextureInfo = materialData.pbrMetallicRoughness.baseColorTexture;
		LoadTexture(baseColorTextureInfo, m_baseColorTexture, m_baseColorTexCoordIndex);
		
		auto& emissiveTextureInfo = materialData.emissiveTexture;
		LoadTexture(emissiveTextureInfo, m_emissiveTexture, m_emissiveTexCoordIndex);

		auto& normalTextureInfo = materialData.normalTexture;
		LoadTexture(normalTextureInfo, m_normalTexture, m_normalTexCoordIndex);

		// TODO: pack if different
		auto& metallicRougnessTextureInfo = materialData.pbrMetallicRoughness.metallicRoughnessTexture;
		auto& occlusionTextureInfo = materialData.occlusionTexture;

		// same texture no need of packing
		//if(metallicRougnessTextureInfo.index == occlusionTextureInfo.index)
		{
			LoadTexture(metallicRougnessTextureInfo, m_occlusionMetallicRoughnessTexture, m_occlusionMetallicRoughnessTexCoordIndex);
		}
	}
	
	return true;
}

void Material::Unload()
{
}
