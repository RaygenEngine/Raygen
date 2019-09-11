#pragma once

#include "assets/Asset.h"

class StringFile : public Asset
{
	std::string m_data;

public:
	StringFile(const fs::path& path)
		: Asset(path) {}
	~StringFile() = default;

	const std::string& GetFileData() const { return m_data; }

protected:
	bool Load() override;
	void Unload() override;
};

