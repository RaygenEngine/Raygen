#include "pch.h"
#include "asset/assets/ShaderAsset.h"
#include "asset/AssetManager.h"

bool ShaderAsset::Load()
{
	std::ifstream t(m_uri);

	if (!t.is_open())
	{
		LOG_WARN("Unable to open string file, path: {}", m_uri);
		return false;
	}

	char buf[256];
	t.getline(buf, 256);
	m_frag = Engine::GetAssetManager()->RequestSearchAsset<TextAsset>(buf);
	
	t.getline(buf, 256);
	m_vert = Engine::GetAssetManager()->RequestSearchAsset<TextAsset>(buf);

	return true;
}
