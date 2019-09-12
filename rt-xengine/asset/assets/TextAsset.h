#pragma once

#include "asset/Asset.h"

class TextAsset : public Asset
{
	std::string m_data;

public:
	TextAsset(const fs::path& path)
		: Asset(path) {}
	~TextAsset() = default;

	const std::string& GetFileData() const { return m_data; }

protected:
	bool Load() override;
	void Unload() override;
};

