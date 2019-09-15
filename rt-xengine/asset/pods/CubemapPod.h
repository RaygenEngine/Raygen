#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/pods/TexturePod.h"

struct CubemapPod : AssetPod
{
	static bool Load(CubemapPod* pod, const fs::path& path);

	PodHandle<TexturePod> sides[CMF_COUNT];
};

