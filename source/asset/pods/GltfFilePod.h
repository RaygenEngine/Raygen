#pragma once

#include "asset/AssetPod.h"
#include "reflection/GenMacros.h"
#include <tiny_gltf.h>

struct GltfFilePod : AssetPod {
	REFLECTED_POD(GltfFilePod) {}
	static void Load(PodEntry* entry, GltfFilePod* pod, const uri::Uri& path);

	tinygltf::Model data;
};
