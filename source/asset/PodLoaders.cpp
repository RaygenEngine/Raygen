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

#include "asset/UriLibrary.h"
#include "system/Logger.h"

void GltfFilePod::Load(GltfFilePod* pod, const uri::Uri& path)
{
	GltfFileLoader::Load(pod, path);
}

void ImagePod::Load(ImagePod* pod, const uri::Uri& path)
{
	if (uri::IsCpu(path)) {
		CustomLoader::Load(pod, path);
		return;
	}
	ImageLoader::Load(pod, path);
}

void MaterialPod::Load(MaterialPod* pod, const uri::Uri& path)
{
	if (uri::MatchesExtension(path, ".gltf")) {
		GltfMaterialLoader::Load(pod, path);
		return;
	}
	CustomLoader::Load(pod, path);
}

void ModelPod::Load(ModelPod* pod, const uri::Uri& path)
{
	if (uri::MatchesExtension(path, ".gltf")) {
		return GltfModelLoader::Load(pod, path);
	}

	LOG_ABORT("Unknown model file found: {}", path);
}

void ShaderPod::Load(ShaderPod* pod, const uri::Uri& path)
{
	return ShaderLoader::Load(pod, path);
}

void StringPod::Load(StringPod* pod, const uri::Uri& path)
{
	return TextLoader::Load(pod, path);
}

void TexturePod::Load(TexturePod* pod, const uri::Uri& path)
{
	if (uri::MatchesExtension(path, ".gltf")) {
		GltfTextureLoader::Load(pod, path);
		return;
	}
	if (uri::MatchesExtension(path, ".json")) {
		CubemapLoader::Load(pod, path);
		return;
	}

	CustomLoader::Load(pod, path);
}

void JsonDocPod::Load(JsonDocPod* pod, const uri::Uri& path)
{
	JsonDocLoader::Load(pod, path);
}
