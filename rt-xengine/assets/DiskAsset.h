#pragma once

#include "assets/Asset.h"

namespace Assets
{
	// File assets loaded from the disk
	class DiskAsset : public Asset
	{
	protected:
		// file path
		std::string m_directoryPath;
		std::string m_filePath;
		std::string m_fileName;

	public:
		DiskAsset(EngineObject* pObject, const std::string& path);
		virtual ~DiskAsset() = default;

		std::string GetDirectoryPath() const { return m_directoryPath; }
		std::string GetFilePath() const { return m_filePath; }
		std::string GetFileName() const { return m_fileName; }

		void ToString(std::ostream& os) const override { os << "asset-type: DiskAsset, name: " << m_name; }
	};
}
