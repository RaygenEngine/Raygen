#include "pch.h"
#include "DiskAsset.h"

#include "DiskAssetManager.h"

namespace Assets
{
	DiskAsset::DiskAsset(DiskAssetManager* diskAssetManager)
		: Asset(diskAssetManager),
		  m_manager(diskAssetManager)
	{
	}

	void DiskAsset::SetIdentificationFromPath(const std::string& path)
	{
		m_path = path;
		m_label = PathSystem::GetNameWithExtension(m_path);
	}
}
