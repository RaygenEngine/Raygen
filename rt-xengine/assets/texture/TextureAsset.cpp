#include "pch.h"

#include "assets/texture/TextureAsset.h"
#include "assets/other/gltf/GltfFileAsset.h"
#include "system/Engine.h"
#include "assets/AssetManager.h"
#include "assets/other/gltf/GltfAux.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "tinygltf/tiny_gltf.h"

bool TextureAsset::Load()
{
	auto finalPath = m_uri;
	
	// TODO check if sub asset
	// 
	// if sub asset
	const auto parentAssetPath = m_uri.parent_path();

	// gltf parent TODO: use a loader
	if (parentAssetPath.extension().compare(".gltf") == 0)
	{
		GltfFileAsset* gltfFile = Engine::GetAssetManager()->RequestAsset<GltfFileAsset>(parentAssetPath);
		if (!Engine::GetAssetManager()->Load(gltfFile))
			return false;

		auto gltfData = gltfFile->GetGltfData();
		const auto ext = m_uri.extension();
		const auto index = std::stoi(&ext.string()[1]);

		auto& textureData = gltfData->textures.at(index);

		const auto imageIndex = textureData.source;

		// if image exists
		if (imageIndex != -1)
		{
			// TODO check image settings
			auto& gltfImage = gltfData->images.at(imageIndex);

			finalPath = parentAssetPath.parent_path() / gltfImage.uri;
		}

		const auto samplerIndex = textureData.sampler;

		// if sampler exists
		if (samplerIndex != -1)
		{
			auto& gltfSampler = gltfData->samplers.at(samplerIndex);

			m_minFilter = GltfAux::GetTextureFiltering(gltfSampler.minFilter);
			m_magFilter = GltfAux::GetTextureFiltering(gltfSampler.magFilter);
			m_wrapS = GltfAux::GetTextureWrapping(gltfSampler.wrapS);
			m_wrapT = GltfAux::GetTextureWrapping(gltfSampler.wrapT);
			m_wrapR = GltfAux::GetTextureWrapping(gltfSampler.wrapR);
		}
		//else keep default values


	}

	m_hdr = stbi_is_hdr(finalPath.string().c_str()) == 1;

	if (!m_hdr)
		m_data = stbi_load(finalPath.string().c_str(), &m_width, &m_height, &m_components, STBI_rgb_alpha);
	else
		m_data = stbi_loadf(finalPath.string().c_str(), &m_width, &m_height, &m_components, STBI_rgb_alpha);

	if (!m_data || (m_width == 0) || (m_height == 0))
	{
		LOG_WARN("TextureAsset loading failed, filepath: {}, data_empty: {} width: {} height: {}", finalPath,
			static_cast<bool>(m_data), m_width, m_height);

		return false;
	}

	return true;
}

void TextureAsset::Unload()
{
	free(m_data);
}
