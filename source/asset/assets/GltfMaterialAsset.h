#pragma once

#include "asset/Asset.h"
#include "asset/pods/MaterialPod.h"

#include "tinygltf/tiny_gltf.h"

struct GltfMaterialAsset
{
	static bool Load(MaterialPod* pod, const fs::path& path);
};
