#include "pch.h"

#include "assets/texture/CubeMap.h"
#include "system/Engine.h"
#include "assets/AssetManager.h"

bool CubeMap::Load()
{
	const fs::path without_ext = m_uri.filename();

	for (int32 i = 0; i < CMF_COUNT; ++i)
	{
		fs::path textPath = without_ext;

		textPath += std::vector({
			"_rt",
			"_lf",
			"_up",
			"_dn",
			"_ft",
			"_bk"
		})[i];

		textPath += m_uri.extension();

		m_faces[i] = Engine::GetAssetManager()->MaybeGenerateAsset<Texture>(textPath);
		if (!Engine::GetAssetManager()->Load(m_faces[i]))
			return false;

		// every texture must match the right face
		if(i == CMF_RIGHT)
		{
			m_width = m_faces[i]->GetWidth();
			m_height = m_faces[i]->GetHeight();
			m_hdr = m_faces[i]->IsHdr();
		}
		// all texture must have same w/h/hdr status
		else if (m_width != m_faces[i]->GetWidth() ||
			     m_height != m_faces[i]->GetHeight() ||
			     m_hdr != m_faces[i]->IsHdr())
			return false; // failed
	}

	return true;
}

void CubeMap::Unload()
{
	//for (auto face : m_faces)
	//face->Unload();
}
