#pragma once

#include "asset/Asset.h"
#include "asset/pods/ImagePod.h"

class ImageAsset : public PodedAsset<ImagePod>
{
public:
	ImageAsset(const fs::path& path)
		: PodedAsset(path) {}

	static ImageAsset* GetDefaultWhite();
	static ImageAsset* GetDefaultMissing();

protected:
	bool Load() override;
	void Deallocate() override;
};