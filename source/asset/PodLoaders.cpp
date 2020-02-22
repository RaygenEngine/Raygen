#include "pch/pch.h"

#include "asset/PodIncludes.h"

#include "asset/loaders/CubemapLoader.h"
#include "asset/loaders/DummyLoader.h"
#include "asset/loaders/GltfFileLoader.h"
#include "asset/loaders/GltfMaterialLoader.h"
#include "asset/loaders/GltfModelLoader.h"
#include "asset/loaders/GltfTextureLoader.h"
#include "asset/loaders/ImageLoader.h"
#include "asset/loaders/ShaderLoader.h"
#include "asset/loaders/TextLoader.h"
#include "asset/loaders/JsonDocLoader.h"
#include "asset/loaders/JsonGenericLoader.h"
#include "asset/loaders/BinaryLoader.h"

#include "asset/UriLibrary.h"
#include "system/Logger.h"

void GltfFilePod::Load(PodEntry* entry, GltfFilePod* pod, const uri::Uri& path)
{
	GltfFileLoader::Load(entry, pod, path);
}

void ImagePod::Load(PodEntry* entry, ImagePod* pod, const uri::Uri& path)
{
	if (uri::IsCpu(path)) {
		CustomLoader::Load(entry, pod, path);
		return;
	}
	ImageLoader::Load(pod, path);
}

void MaterialPod::Load(PodEntry* entry, MaterialPod* pod, const uri::Uri& path)
{
	if (uri::MatchesExtension(path, ".gltf")) {
		GltfMaterialLoader::Load(pod, path);
		return;
	}
	if (uri::MatchesExtension(path, ".json")) {
		GenericJsonLoader::Load(pod, path);
		return;
	}
	CustomLoader::Load(entry, pod, path);
}

void ModelPod::Load(PodEntry* entry, ModelPod* pod, const uri::Uri& path)
{
	if (uri::MatchesExtension(path, ".gltf")) {
		return GltfModelLoader::Load(entry, pod, path);
	}
}

void ShaderPod::Load(PodEntry* entry, ShaderPod* pod, const uri::Uri& path)
{
	return ShaderLoader::Load(pod, path);
}

void StringPod::Load(PodEntry* entry, StringPod* pod, const uri::Uri& path)
{
	return TextLoader::Load(pod, path);
}

void BinaryPod::Load(PodEntry* entry, BinaryPod* pod, const uri::Uri& path)
{
	return BinaryLoader::Load(pod, path);
}

void TexturePod::Load(PodEntry* entry, TexturePod* pod, const uri::Uri& path)
{
	if (uri::MatchesExtension(path, ".gltf")) {
		GltfTextureLoader::Load(entry, pod, path);
		return;
	}
	if (uri::MatchesExtension(path, ".json")) {
		if (!CubemapLoader::Load(pod, path)) {
			GenericJsonLoader::Load(pod, path);
		}
		return;
	}

	CustomLoader::Load(entry, pod, path);
}

void JsonDocPod::Load(PodEntry* entry, JsonDocPod* pod, const uri::Uri& path)
{
	JsonDocLoader::Load(entry, pod, path);
}
