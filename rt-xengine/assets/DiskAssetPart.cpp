#include "pch.h"

#include "assets/DiskAssetPart.h"
#include "assets/PathSystem.h"

namespace Assets
{
	DiskAssetPart::DiskAssetPart(DiskAsset* pAsset, const std::string& name)
		: DiskAsset(pAsset, pAsset->GetFilePath() + "$" + name)
	{
	}

	void DiskAssetPart::RenameAssetPart(const std::string& newName)
	{
		const auto pAsset = dynamic_cast<DiskAsset*>(GetParentObject());
		const auto newDesc = pAsset->GetFilePath() + "$" + newName;
		
		m_directoryPath = PathSystem::GetParentPath(newDesc);
		m_filePath = newDesc;
		m_fileName = PathSystem::GetNameWithExtension(newDesc);
		m_name = PathSystem::GetNameWithExtension(newDesc);
	}
}
