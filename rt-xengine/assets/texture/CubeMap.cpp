#include "pch.h"

#include "assets/texture/CubeMap.h"
#include "assets/AssetManager.h"
#include "system/Engine.h"

bool CubeMap::LoadFaceTexture(CubeMapFace index, const std::string& path)
{
	m_faces[index] = Engine::GetAssetManager()->LoadTextureAsset(path);

	if (!m_faces[index])
		return false; // failed
	
	std::call_once(m_initFlag, [&]()
		{
			m_width = m_faces[index]->GetWidth();
			m_height = m_faces[index]->GetHeight();
			m_hdr = m_faces[index]->IsHdr();
		});

	// all texture must have same w/h/hdr status
	if (m_width != m_faces[index]->GetWidth()   ||
		m_height != m_faces[index]->GetHeight() ||
		m_hdr != m_faces[index]->IsHdr())
		return false; // failed

	return true;
}
