#pragma once

#include "asset/Asset.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/MaterialPod.h"

class DefaultTexture : public PodedAsset<TexturePod>
{
public:
	DefaultTexture(const fs::path& path)
		: PodedAsset(path) {}
	~DefaultTexture() = default;

	static DefaultTexture* GetDefault();
	
protected:
	bool Load() override;
};

class DefaultMaterial : public PodedAsset<MaterialPod>
{
public:
	DefaultMaterial(const fs::path& path)
		: PodedAsset(path) {}
	~DefaultMaterial() = default;

	static DefaultMaterial* GetDefault();
	
protected:
	bool Load() override;
};
