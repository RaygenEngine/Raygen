#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"

#include "tinygltf/tiny_gltf.h"

struct GltfFilePod : AssetPod
{
	tinygltf::Model data;
};

