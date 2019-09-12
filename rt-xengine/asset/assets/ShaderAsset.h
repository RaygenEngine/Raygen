#pragma once

#include "asset/assets/TextAsset.h"

class ShaderAsset : public Asset
{
public:
	TextAsset* m_vert;
	TextAsset* m_frag;

	ShaderAsset(const fs::path& path)
		: Asset(path) {}

protected:
	bool Load() override;
	void Unload() override { }
};

