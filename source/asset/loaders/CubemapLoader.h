#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/CubemapPod.h"

namespace CubemapLoader
{
	inline bool Load(CubemapPod* pod, const fs::path& path)
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

			pod->sides[i]->image = AssetManager::GetOrCreate<ImagePod>(name);
		}

		return true;
	}
};
