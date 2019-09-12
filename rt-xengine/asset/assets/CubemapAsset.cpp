#include "pch.h"

#include "asset/assets/CubemapAsset.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
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

		

		m_sides[i].image = Engine::GetAssetManager()->RequestSearchAsset<ImageAsset>(std::string(name));
		
		// every texture must match the right face
		if (i == CMF_RIGHT)
		{
			m_width = m_sides[i].image->GetWidth();
			m_height = m_sides[i].image->GetHeight();
			m_hdr = m_sides[i].image->IsHdr();
		}
		// TODO: remove limitation
		// all texture must have same w/h/hdr status
		else if (m_width != m_sides[i].image->GetWidth() ||
				 m_height != m_sides[i].image->GetHeight() ||
				 m_hdr != m_sides[i].image->IsHdr())
		{
			return false; // failed
		}
	}

	return true;
}

void CubemapAsset::Unload()
{

}
