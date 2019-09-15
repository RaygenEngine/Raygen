#pragma once

#include "asset/Asset.h"
#include "asset/pods/TextPod.h"

struct TextAsset
{
	static bool Load(TextPod* pod, const fs::path& path);
};

