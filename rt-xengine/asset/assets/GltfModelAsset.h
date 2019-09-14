#pragma once

#include "asset/Asset.h"
#include "asset/pods/ModelPod.h"

#include "tinygltf/tiny_gltf.h"

class GltfModelAsset : public PodedAsset<ModelPod>
{
public:
	
	GltfModelAsset(const fs::path& path)
		: PodedAsset(path) {}

protected:
	bool Load() override;
};
