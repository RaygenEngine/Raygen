#ifndef DISKASSET_H
#define DISKASSET_H

#include "Asset.h"

namespace Assets
{
	class DiskAssetManager;

	// File assets loaded from the disk
	class DiskAsset : public Asset
	{
	protected:
		// file path
		std::string m_path;
		std::string m_label;

		// manager than manages this asset
		DiskAssetManager* m_manager;

	public:
		DiskAsset(DiskAssetManager* diskAssetManager);
		virtual ~DiskAsset() = default;

		void SetIdentificationFromPath(const std::string& path);

		// absolute path <unique>
		std::string GetPath() const { return m_path; }
		std::string GetLabel() const { return m_label; }
	};
}

#endif // DISKASSET_H
