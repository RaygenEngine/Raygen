#pragma once

#include "asset/Asset.h"
#include "asset/pods/GltfFilePod.h"

class GltfFileAsset : public PodedAsset<GltfFilePod>
{
public:
	
	GltfFileAsset(const fs::path& path)
		: PodedAsset(path) {}

protected:
	bool Load() override;
};
