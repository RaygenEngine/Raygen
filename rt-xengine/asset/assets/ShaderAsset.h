#pragma once

#include "asset/assets/TextAsset.h"
#include "asset/pods/ShaderPod.h"

class ShaderAsset : public PodedAsset<ShaderPod>
{
public:
	ShaderAsset(const fs::path& path)
		: PodedAsset(path) {}
	
protected:
	bool Load() override;
};

