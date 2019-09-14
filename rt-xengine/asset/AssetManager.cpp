#include "pch.h"

#include "asset/AssetManager.h"
#include "asset/assets/ImageAsset.h"

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	return m_pathSystem.Init(applicationPath, dataDirectoryName);
}
