#include "pch.h"

#include "asset/assets/ShaderAsset.h"
#include "asset/AssetManager.h"

bool ShaderAsset::Load(ShaderPod* pod, const fs::path& path)
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
