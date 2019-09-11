#include "pch.h"

#include "assets/AssetManager.h"

bool AssetManager::Init(const std::string& applicationPath, const std::string& dataDirectoryName)
{
	return m_pathSystem.Init(applicationPath, dataDirectoryName);
}
