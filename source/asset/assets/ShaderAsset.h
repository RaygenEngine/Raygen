#pragma once

#include "asset/assets/TextAsset.h"
#include "asset/pods/ShaderPod.h"

struct ShaderAsset
{
	static bool Load(ShaderPod* pod, const fs::path& path);
};

