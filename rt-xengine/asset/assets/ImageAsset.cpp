#include "pch.h"

#include "asset/assets/ImageAsset.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "stb_image/stb_image.h"

bool ImageAsset::Load()
{
	auto finalPath = m_uri;
	
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

void ImageAsset::Unload()
{
	free(m_data);
}
