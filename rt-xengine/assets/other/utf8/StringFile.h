#ifndef STRINGFILE_H
#define STRINGFILE_H

#include "assets/DiskAsset.h"

#include <string>

namespace Assets
{
	class DiskAssetManager;

	class StringFile : public DiskAsset
	{
		std::string m_data;

	public:
		StringFile(DiskAssetManager* context);
		~StringFile() = default;

		bool Load(const std::string& path);
		void Clear() override;

		const std::string& GetFileData() const { return m_data; }

		void ToString(std::ostream& os) const override { os << "type: StringFile, name: " << m_label; }
	};
}

#endif // STRINGFILE_H
