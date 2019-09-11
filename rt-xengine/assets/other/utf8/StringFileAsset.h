#pragma once

#include "assets/Asset.h"

class StringFileAsset : public Asset
{
	std::string m_data;

public:
	StringFileAsset(const fs::path& path)
		: Asset(path) {}
	~StringFileAsset() = default;

	const std::string& GetFileData() const { return m_data; }

protected:
	bool Load() override;
	void Unload() override;
};

