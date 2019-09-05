#pragma once

#include "assets/Asset.h"

class StringFile : public Asset
{
	std::string m_data;

public:
	StringFile(AssetManager* assetManager, const std::string& path)
		: Asset(assetManager, path) {}
	~StringFile() = default;

	bool Load(const std::string& path);
	void Clear() override;

	const std::string& GetFileData() const { return m_data; }

	void ToString(std::ostream& os) const override { os << "type: StringFile, name: " << m_fileName; }
};

