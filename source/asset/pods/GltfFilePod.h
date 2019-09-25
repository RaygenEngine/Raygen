#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"

#include "tinygltf/tiny_gltf.h"

struct GltfFilePod : public AssetPod
{
	STATIC_REFLECTOR(GltfFilePod)
	{

	}
	static bool Load(GltfFilePod* pod, const fs::path& path);

	tinygltf::Model data;
};

