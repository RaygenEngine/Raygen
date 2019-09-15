#pragma once

#include "asset/Asset.h"
#include "asset/pods/GltfFilePod.h"

struct GltfFileAsset 
{
	static bool Load(GltfFilePod* pod, const fs::path& path);
};
