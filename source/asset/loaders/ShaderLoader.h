#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/ShaderPod.h"
#include "asset/UriLibrary.h"

namespace ShaderLoader
{
	inline bool Load(ShaderPod* pod, const uri::Uri& path)
	{
		std::ifstream t(uri::ToSystemPath(path));

		if (!t.is_open())
		{
			LOG_WARN("Unable to open string file, path: {}", uri::ToSystemPath(path));
			return false;
		}

		char buf[256];

		t.getline(buf, 256);
		pod->vertex = AssetManager::GetOrCreateFromParentUri<StringPod>(buf, path);

		t.getline(buf, 256);
		pod->fragment = AssetManager::GetOrCreateFromParentUri<StringPod>(buf, path);

		return true;
	}

};

