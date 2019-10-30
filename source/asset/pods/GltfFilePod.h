#pragma once

#include "asset/AssetPod.h"

#include <tiny_gltf.h>

struct GltfFilePod : AssetPod {
	REFLECTED_POD(GltfFilePod) {}
	static void Load(GltfFilePod* pod, const uri::Uri& path);

	tinygltf::Model data;
};
