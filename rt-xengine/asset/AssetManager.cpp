#include "pch.h"

#include "asset/AssetManager.h"
#include "asset/assets/ImageAsset.h"

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	auto ret = m_pathSystem.Init(applicationPath, dataDirectoryName);

	const auto textDefault = RequestSearchAsset<ImageAsset>(__default__textureWhite);
	ret = ret && Load(textDefault);

	const auto textMissing = RequestSearchAsset<ImageAsset>(__default__textureMissing);
	ret = ret && Load(textMissing);
	
	return ret;
}

ImageAsset* AssetManager::GetDefaultWhite()
{
	// PERF
	return Engine::GetAssetManager()->RequestSearchAsset<ImageAsset>(__default__textureWhite);
}

ImageAsset* AssetManager::GetDefaultMissing()
{
	// PERF
	return Engine::GetAssetManager()->RequestSearchAsset<ImageAsset>(__default__textureMissing);
}