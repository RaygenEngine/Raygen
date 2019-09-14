#include "pch.h"

#include "asset/assets/ImageAsset.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image/stb_image.h"
#include "tinygltf/tiny_gltf.h"

ImageAsset* ImageAsset::GetDefaultWhite()
{
	return Engine::GetAssetManager()->RequestSearchAsset<ImageAsset>(__default__imageWhite);
}

ImageAsset* ImageAsset::GetDefaultMissing()
{
	return Engine::GetAssetManager()->RequestSearchAsset<ImageAsset>(__default__imageMissing);
}

bool ImageAsset::Load()
{
	auto finalPath = m_uri;
	
	m_pod->hdr = stbi_is_hdr(finalPath.string().c_str()) == 1;

	if (!m_pod->hdr)
		m_pod->data = stbi_load(finalPath.string().c_str(), &m_pod->width, &m_pod->height, &m_pod->components, STBI_rgb_alpha);
	else
		m_pod->data = stbi_loadf(finalPath.string().c_str(), &m_pod->width, &m_pod->height, &m_pod->components, STBI_rgb_alpha);

	if (!m_pod->data || (m_pod->width == 0) || (m_pod->height == 0))
	{
		LOG_WARN("TextureAsset loading failed, filepath: {}, data_empty: {} width: {} height: {}", finalPath,
			static_cast<bool>(m_pod->data), m_pod->width, m_pod->height);

		return false;
	}

	return true;
}

void ImageAsset::Deallocate()
{
	// TODO: check
	free(m_pod->data);
	PodedAsset::Deallocate();
}
