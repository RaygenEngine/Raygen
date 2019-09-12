#include "pch.h"

#include "assets/texture/CubeMapAsset.h"
#include "system/Engine.h"
#include "assets/AssetManager.h"
#include <iostream>

bool CubemapAsset::Load()
{
	std::ifstream t(m_uri);

	if (!t.is_open())
	{
		LOG_WARN("Unable to open string file, path: {}", m_uri);
		return false;
	}


	for (int32 i = 0; i < CMF_COUNT; ++i)
	{
		char name[256];
		t.getline(name, 256);

		m_faces[i] = Engine::GetAssetManager()->RequestSearchAsset<TextureAsset>(std::string(name));
		
		// every texture must match the right face
		if (i == CMF_RIGHT)
		{
			m_width = m_faces[i]->GetWidth();
			m_height = m_faces[i]->GetHeight();
			m_hdr = m_faces[i]->IsHdr();
		}
		// TODO: remove limitation
		// all texture must have same w/h/hdr status
		else if (m_width != m_faces[i]->GetWidth() ||
				 m_height != m_faces[i]->GetHeight() ||
				 m_hdr != m_faces[i]->IsHdr())
		{
			return false; // failed
		}
	}

	return true;
}

void CubemapAsset::Unload()
{

}
