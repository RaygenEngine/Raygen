#include "pch.h"

#include "assets/texture/CubeMap.h"
#include "assets/AssetManager.h"

bool CubeMap::Load(const std::string& path, DynamicRange dr, bool flipVertically)
{
	// TODO from system
	auto onePath = PathSystem::GetPathWithoutExtension(path);
	const auto generalPath = onePath.substr(0, onePath.size() - 3);

	const auto extension = PathSystem::GetExtension(path);

	std::string paths[CMF_COUNT];

	paths[CMF_RIGHT] = generalPath + "_rt" + extension;
	paths[CMF_LEFT]  = generalPath + "_lf" + extension;
	paths[CMF_UP]    = generalPath + "_up" + extension;
	paths[CMF_DOWN]  = generalPath + "_dn" + extension;
	paths[CMF_FRONT] = generalPath + "_ft" + extension;
	paths[CMF_BACK]  = generalPath + "_bk" + extension;

	for(auto i = 0; i < CMF_COUNT; ++i)
	{
		// paths here will be in absolute format (already calculated for the cube-map itself)
		m_faces[i] = GetAssetManager()->LoadTextureAsset(paths[i], dr, flipVertically);

		if(!m_faces[i])
			return false; // failed

		// all texture must have same w/h
		if (m_width == 0)
			m_width = m_faces[i]->GetWidth();
		else if (m_width != m_faces[i]->GetWidth())
			return false; // failed

		if(m_height == 0)
			m_height = m_faces[i]->GetHeight();
		else if (m_height != m_faces[i]->GetHeight())
			return false; // failed

	}

	return true;
}

void CubeMap::Clear()
{
	for (auto i = 0; i < CMF_COUNT; ++i)
		m_faces[i]->Unload();
}

