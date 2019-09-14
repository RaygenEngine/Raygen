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
	auto textAsset = Engine::GetAssetManager()->RequestSearchAsset<TextAsset>(buf);
	m_pod->vertex = textAsset->GetPod();
	
	t.getline(buf, 256);
	textAsset = Engine::GetAssetManager()->RequestSearchAsset<TextAsset>(buf);
	m_pod->fragment = textAsset->GetPod();

	return true;
}
