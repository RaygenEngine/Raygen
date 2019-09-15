#pragma once

#include "asset/Asset.h"
#include "asset/pods/CubemapPod.h"

struct CubemapAsset
{
	static bool Load(CubemapPod* pod, const fs::path& path);
};
