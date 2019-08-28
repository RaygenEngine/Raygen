#pragma once

#include "assets/DiskAsset.h"

namespace Assets
{
	// File assets loaded from the disk
	class DiskAssetPart : public DiskAsset
	{
	public:
		DiskAssetPart(DiskAsset* pAsset, const std::string& name);
		virtual ~DiskAssetPart() = default;

		// asset parts can be renamed
		void RenameAssetPart(const std::string& name);

		void ToString(std::ostream& os) const override { os << "asset-type: DiskAssetPart, name: " << m_name; }
	};
}
