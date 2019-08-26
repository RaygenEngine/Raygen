#pragma once

#include "assets/DiskAsset.h"

namespace Assets
{
	class DiskAssetManager;

	class StringFile : public DiskAsset
	{
		std::string m_data;

	public:
		StringFile(DiskAssetManager* context, const std::string& path);
		~StringFile() = default;

		bool Load(const std::string& path);
		void Clear() override;

		const std::string& GetFileData() const { return m_data; }

		void ToString(std::ostream& os) const override { os << "type: StringFile, name: " << m_name; }
	};
}
