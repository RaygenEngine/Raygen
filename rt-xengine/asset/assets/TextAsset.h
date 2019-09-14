#pragma once

#include "asset/Asset.h"
#include "asset/pods/TextPod.h"

class TextAsset : public PodedAsset<TextPod>
{
public:
	TextAsset(const fs::path& path)
		: PodedAsset(path) {}
	~TextAsset() = default;
	
protected:
	bool Load() override;
};

