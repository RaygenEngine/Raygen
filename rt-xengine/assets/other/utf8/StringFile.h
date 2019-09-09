#pragma once

#include "assets/FileAsset.h"

class StringFile : public FileAsset
{
	std::string m_data;

public:
	StringFile(const std::string& path)
		: FileAsset(path) {}
	~StringFile() = default;

	bool Load(const std::string& path);
	void Clear() override;

	const std::string& GetFileData() const { return m_data; }

	void ToString(std::ostream& os) const override { os << "type: StringFile, name: " << m_fileName; }
};

