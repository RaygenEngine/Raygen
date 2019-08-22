#ifndef DISKASSETPART_H
#define DISKASSETPART_H

#include "DiskAsset.h"

#include "DiskAssetManager.h"

namespace Assets
{
	// Assets that are part of DiskAssets (loaded from data provided by the disk asset)
	class DiskAssetPart : public Asset
	{
	protected:
		DiskAsset* m_parent;
		std::string m_label;

	public:
		DiskAssetPart(DiskAsset* parent)
			: Asset(parent->GetDiskAssetManager()),
			m_parent(parent)
		{
		}
		virtual ~DiskAssetPart() = default;

		// absolute path <unique>
		DiskAsset* GetParent() const { return m_parent; }
		std::string GetLabel() const { return m_label; }
	};
}

#endif // DISKASSETPART_H
