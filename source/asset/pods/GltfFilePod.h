#pragma once

#include "asset/AssetPod.h"

#include <tinygltf/tiny_gltf.h>

struct GltfFilePod : AssetPod {
	REFLECTED_POD(GltfFilePod) {}
	static bool Load(GltfFilePod* pod, const uri::Uri& path);

	tinygltf::Model data;
};
