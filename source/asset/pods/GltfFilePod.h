#pragma once

#include "reflection/GenMacros.h"
#include "asset/AssetPod.h"

#include <tinygltf/tiny_gltf.h>

struct GltfFilePod : public AssetPod {
	REFLECTED_POD(GltfFilePod) {}
	static void Load(GltfFilePod* pod, const uri::Uri& path);

	tinygltf::Model data;
};
