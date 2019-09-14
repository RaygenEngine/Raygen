#include "pch.h"

#include "asset/assets/GltfTextureAsset.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "asset/assets/GltfFileAsset.h"
#include "asset/assets/ImageAsset.h"
#include "asset/util/GltfAux.h"

bool GltfTextureAsset::Load()
{
	const auto pPath = m_uri.parent_path();
	auto pParent = Engine::GetAssetManager()->RequestSearchAsset<GltfFileAsset>(pPath);

	// requires parent to be loaded
	if (!Engine::GetAssetManager()->Load(pParent))
		return false;

	const auto info = m_uri.filename();
	const auto ext = std::stoi(&info.extension().string()[1]);

	tinygltf::Model& model = pParent->GetPod()->data;
	
	auto& gltfTexture = model.textures.at(ext);

	const auto imageIndex = gltfTexture.source;

	auto imgAsset = ImageAsset::GetDefaultWhite();
	
	// if image exists
	if (imageIndex != -1)
	{
		// TODO check image settings
		auto& gltfImage = model.images.at(imageIndex);

		auto textPath = pPath.parent_path() / gltfImage.uri;
		
		imgAsset = Engine::GetAssetManager()->RequestSearchAsset<ImageAsset>(textPath);
	}
	
	m_pod->image = imgAsset->GetPod();

	const auto samplerIndex = gltfTexture.sampler;
	// if sampler exists
	if (samplerIndex != -1)
	{
		auto& gltfSampler = model.samplers.at(samplerIndex);

		m_pod->minFilter = GltfAux::GetTextureFiltering(gltfSampler.minFilter);
		m_pod->magFilter = GltfAux::GetTextureFiltering(gltfSampler.magFilter);
		m_pod->wrapS = GltfAux::GetTextureWrapping(gltfSampler.wrapS);
		m_pod->wrapT = GltfAux::GetTextureWrapping(gltfSampler.wrapT);
		m_pod->wrapR = GltfAux::GetTextureWrapping(gltfSampler.wrapR);
	}
	
	//else keep default values
	return true;
}
