#pragma once

#include "asset/Asset.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/MaterialPod.h"

struct DefaultTexture
{
	static PodHandle<TexturePod> GetDefault();
	static bool Load(TexturePod* pod, const fs::path& path);
};

struct DefaultMaterial
{
	static PodHandle<MaterialPod> GetDefault();
	static bool Load(MaterialPod* pod, const fs::path& path);
};
