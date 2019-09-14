#include "pch.h"

#include "asset/assets/GltfMaterialAsset.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "asset/assets/GltfFileAsset.h"
#include "asset/util/GltfAux.h"
#include "asset/assets/GltfTextureAsset.h"
#include "asset/assets/DummyAssets.h"

bool GltfMaterialAsset::Load()
{
	const auto pPath = m_uri.parent_path();
	auto pParent = Engine::GetAssetManager()->RequestSearchAsset<GltfFileAsset>(pPath);

	// requires parent to be loaded
	if (!Engine::GetAssetManager()->Load(pParent))
		return false;

	const auto info = m_uri.filename();
	const auto ext = std::stoi(&info.extension().string()[1]);

	tinygltf::Model& model = pParent->GetPod()->data;

	auto& gltfMaterial = model.materials.at(ext);
	
	// factors
	auto bFactor = gltfMaterial.pbrMetallicRoughness.baseColorFactor;
	m_pod->baseColorFactor = { bFactor[0], bFactor[1], bFactor[2], bFactor[3] };
	m_pod->metallicFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.metallicFactor);
	m_pod->roughnessFactor = static_cast<float>(gltfMaterial.pbrMetallicRoughness.roughnessFactor);
	auto eFactor = gltfMaterial.emissiveFactor;
	m_pod->emissiveFactor = { eFactor[0], eFactor[1], eFactor[2] };

	// scales/strenghts
	m_pod->normalScale = static_cast<float>(gltfMaterial.normalTexture.scale);
	m_pod->occlusionStrength = static_cast<float>(gltfMaterial.occlusionTexture.strength);

	// alpha
	m_pod->alphaMode = GltfAux::GetAlphaMode(gltfMaterial.alphaMode);

	m_pod->alphaCutoff = static_cast<float>(gltfMaterial.alphaCutoff);
	// doublesided-ness
	m_pod->doubleSided = gltfMaterial.doubleSided;


	auto LoadTexture = [&](auto textureInfo, TexturePod*& sampler, int32& textCoordIndex, bool useDefaultIfMissing = true)
	{
		if (textureInfo.index != -1)
		{
			tinygltf::Texture& gltfTexture = model.textures.at(textureInfo.index);

			auto textPath = pPath / ("#" + (!gltfTexture.name.empty() ? gltfTexture.name : "sampler") + "." + std::to_string(textureInfo.index));

			sampler = Engine::GetAssetManager()->RequestSearchAsset<GltfTextureAsset>(textPath)->GetPod();

			textCoordIndex = textureInfo.texCoord;
		}
		else
		{
			sampler = DefaultTexture::GetDefault()->GetPod();
		}

		return true;
	};

	// samplers
	auto& baseColorTextureInfo = gltfMaterial.pbrMetallicRoughness.baseColorTexture;
	LoadTexture(baseColorTextureInfo, m_pod->baseColorTexture, m_pod->baseColorTexCoordIndex);

	auto& emissiveTextureInfo = gltfMaterial.emissiveTexture;
	LoadTexture(emissiveTextureInfo, m_pod->emissiveTexture, m_pod->emissiveTexCoordIndex);

	auto& normalTextureInfo = gltfMaterial.normalTexture;
	LoadTexture(normalTextureInfo, m_pod->normalTexture, m_pod->normalTexCoordIndex, false);

	// TODO: pack if different
	auto& metallicRougnessTextureInfo = gltfMaterial.pbrMetallicRoughness.metallicRoughnessTexture;
	auto& occlusionTextureInfo = gltfMaterial.occlusionTexture;

	// same texture no need of packing
	//if(metallicRougnessTextureInfo.index == occlusionTextureInfo.index)
	{
		LoadTexture(metallicRougnessTextureInfo, m_pod->occlusionMetallicRoughnessTexture, m_pod->occlusionMetallicRoughnessTexCoordIndex);
	}
	return true;
}
