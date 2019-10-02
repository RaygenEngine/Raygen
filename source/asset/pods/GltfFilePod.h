#pragma once

#include "core/reflection/GenMacros.h"
#include "asset/AssetPod.h"

#include "tinygltf/tiny_gltf.h"

struct GltfFilePod : public AssetPod
{
	REFLECTED_POD(GltfFilePod)
	{

	}
	static bool Load(GltfFilePod* pod, const fs::path& path);

	tinygltf::Model data;
};

