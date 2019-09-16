#include "pch.h"

#include "asset/AssetManager.h"

size_t AssetManager::NextHandle = 1;

AssetPod* AssetManager::__DebugUid(size_t a)
{
	return Engine::GetAssetManager()->m_uidToPod.at(a);
}

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	return m_pathSystem.Init(applicationPath, dataDirectoryName);
}



