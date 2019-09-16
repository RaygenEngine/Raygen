#include "pch.h"

#include "asset/assets/CubemapAsset.h"
#include "system/Engine.h"
#include "asset/AssetManager.h"
#include "asset/assets/ImageAsset.h"

#include <iostream>

bool CubemapAsset::Load(CubemapPod* pod, const fs::path& path)
{
	std::ifstream t(path);

	if (!t.is_open())
	{
		LOG_WARN("Unable to open string file, path: {}", path);
		return false;
	}

	for (int32 i = 0; i < CMF_COUNT; ++i)
	{
		char name[256];
		t.getline(name, 256);

		pod->sides[i] = AssetManager::GetOrCreate<TexturePod>("#" + std::string(name) + "_" + std::to_string(i));
		pod->sides[i]->image = AssetManager::GetOrCreate<ImagePod>(name);
	}

	return true;
}
