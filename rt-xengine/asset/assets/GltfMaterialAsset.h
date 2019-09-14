#pragma once

#include "asset/Asset.h"
#include "asset/pods/MaterialPod.h"

#include "tinygltf/tiny_gltf.h"

class GltfMaterialAsset : public PodedAsset<MaterialPod>
{
public:
	
	GltfMaterialAsset(const fs::path& path)
		: PodedAsset(path) {}
	
protected:
	bool Load() override;
};
