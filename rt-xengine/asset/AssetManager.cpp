#include "pch.h"

#include "asset/AssetManager.h"
#include "asset/assets/ImageAsset.h"
#include "asset/assets/GltfMaterialAsset.h"

size_t AssetManager::NextHandle = 1;

void AssetManager::Unload(Asset* asset)
{
	if (asset->m_isLoaded && dynamic_cast<GltfMaterialAsset*>(asset) == nullptr)
	{
		asset->Deallocate();
	}

	asset->m_isLoaded = false;
}

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	return m_pathSystem.Init(applicationPath, dataDirectoryName);
}


