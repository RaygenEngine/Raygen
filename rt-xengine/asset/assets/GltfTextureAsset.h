#pragma once

#include "asset/Asset.h"
#include "asset/pods/TexturePod.h"

struct GltfTextureAsset
{
	static bool Load(TexturePod* pod, const fs::path& path);
};
