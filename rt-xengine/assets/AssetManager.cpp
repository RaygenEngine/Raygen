#include "pch.h"

#include "assets/AssetManager.h"
#include "assets/texture/TextureAsset.h"

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	auto ret = m_pathSystem.Init(applicationPath, dataDirectoryName);

	auto path = m_pathSystem.SearchAssetPath(__default__textureWhite);
	const auto textDefault = RequestAsset<TextureAsset>(path);
	ret = ret && Load(textDefault);

	path = m_pathSystem.SearchAssetPath(__default__textureMissing);
	const auto textMissing = RequestAsset<TextureAsset>(path);
	ret = ret && Load(textMissing);
	
	return ret;
}
