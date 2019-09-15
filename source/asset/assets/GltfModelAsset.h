#pragma once

#include "asset/Asset.h"
#include "asset/pods/ModelPod.h"

#include "tinygltf/tiny_gltf.h"

struct GltfModelAsset
{
	static bool Load(ModelPod* pod, const fs::path& path);
};
