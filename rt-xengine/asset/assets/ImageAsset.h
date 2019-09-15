#pragma once

#include "asset/Asset.h"
#include "asset/pods/ImagePod.h"

struct ImageAsset
{
	static bool Load(ImagePod* pod, const fs::path& path);
};