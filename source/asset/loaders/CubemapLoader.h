#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"

namespace CubemapLoader
{
	inline bool Load(TexturePod* pod, const uri::Uri& path)
	{
		std::ifstream t(uri::ToSystemPath(path));

		if (!t.is_open())
		{
			LOG_WARN("Unable to open string file, path: {}", uri::ToSystemPath(path));
			return false;
		}

		for (int32 i = 0; i < CMF_COUNT; ++i)
		{
			char name[256];
			t.getline(name, 256);
			
			pod->target = TextureTarget::TEXTURE_CUBEMAP;
			pod->images.push_back(AssetManager::GetOrCreateFromParentUri<ImagePod>(name, path));
		}

		return true;
	}
};
