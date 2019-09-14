#include "pch.h"

#include "asset/assets/CubemapAsset.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "asset/assets/ImageAsset.h"

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

		auto imgAsset = Engine::GetAssetManager()->RequestSearchAsset<ImageAsset>(std::string(name));
		m_pod->sides[i]->image = imgAsset->GetPod();
	}

	return true;
}
