#include "pch.h"

#include "assets/DiskAsset.h"
#include "assets/DiskAssetManager.h"

namespace Assets
{
	DiskAsset::DiskAsset(EngineObject* pObject, const std::string& path)
	     // asset name of disk assets contains their extension
		: Asset(pObject, PathSystem::GetNameWithExtension(path)),
		  m_directoryPath(PathSystem::GetParentPath(path)),
		  m_filePath(path),
	      m_fileName(PathSystem::GetNameWithExtension(path))
	{
	}
}
