#include "pch.h"

#include "asset/assets/DummyAssets.h"
#include "asset/assets/ImageAsset.h"
#include "asset/AssetManager.h"

PodHandle<TexturePod> DefaultTexture::GetDefault()
{
	return AssetManager::GetOrCreate<TexturePod>(__default__texture);
}

bool DefaultTexture::Load(TexturePod* pod, const fs::path& path)
{
	pod->image = AssetManager::GetOrCreate<ImagePod>(__default__imageWhite);
	return true;
}

PodHandle<MaterialPod> DefaultMaterial::GetDefault()
{
	return AssetManager::GetOrCreate<MaterialPod>(__default__material);
}

bool DefaultMaterial::Load(MaterialPod* pod, const fs::path& path)
{
	auto image = DefaultTexture::GetDefault();
	pod->baseColorTexture = image;
	pod->normalTexture = image;
	pod->emissiveTexture = image;
	pod->occlusionMetallicRoughnessTexture = image;

	return true;
}
