#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/ShaderPod.h"

namespace ShaderLoader
{
	inline bool Load(ShaderPod* pod, const fs::path& path)
	{
		std::ifstream t(path);

		if (!t.is_open())
		{
			LOG_WARN("Unable to open string file, path: {}", path);
			return false;
		}

		char buf[256];

		t.getline(buf, 256);
		pod->vertex = AssetManager::GetOrCreate<TextPod>(buf);

		t.getline(buf, 256);
		pod->fragment = AssetManager::GetOrCreate<TextPod>(buf);

		return true;
	}

};

