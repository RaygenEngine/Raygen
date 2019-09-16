#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/MaterialPod.h"

namespace DefaultTextureLoader
{
	inline PodHandle<TexturePod> GetDefault()
	{
		return AssetManager::GetOrCreate<TexturePod>(__default__texture);
	}
	
	inline bool Load(TexturePod* pod, const fs::path& path)
	{
		pod->image = AssetManager::GetOrCreate<ImagePod>(__default__imageWhite);
		return true;
	}
};

namespace DefaultMaterialLoader
{
	inline PodHandle<MaterialPod> GetDefault()
	{
		return AssetManager::GetOrCreate<MaterialPod>(__default__material);
	}

	inline bool Load(MaterialPod* pod, const fs::path& path)
	{
		const auto image = DefaultTextureLoader::GetDefault();
		pod->baseColorTexture = image;
		pod->normalTexture = image;
		pod->emissiveTexture = image;
		pod->occlusionMetallicRoughnessTexture = image;

		return true;
	}
};
