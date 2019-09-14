#pragma once

#include "asset/Asset.h"
#include "asset/pods/TexturePod.h"

class GltfTextureAsset : public PodedAsset<TexturePod>
{
public:
	
	GltfTextureAsset(const fs::path& path)
		: PodedAsset(path) {}

protected:
	bool Load() override;
};
