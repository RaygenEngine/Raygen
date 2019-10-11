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


bool GltfFilePod::Load(GltfFilePod* pod, const uri::Uri& path)
{
	return GltfFileLoader::Load(pod, path);
}

bool ImagePod::Load(ImagePod* pod, const uri::Uri& path)
{
	if (uri::IsCpu(path)) {
		return CustomLoader::Load(pod, path);
	}
	return ImageLoader::Load(pod, path);
}

bool MaterialPod::Load(MaterialPod* pod, const uri::Uri& path)
{
	if (uri::MatchesExtension(path, ".gltf")) {
		return GltfMaterialLoader::Load(pod, path);
	}
	return CustomLoader::Load(pod, path);
}

bool ModelPod::Load(ModelPod* pod, const uri::Uri& path)
{
	if (uri::MatchesExtension(path, ".gltf")) {
		return GltfModelLoader::Load(pod, path);
	}
	// Add obj loader or others

	return false;
}

bool ShaderPod::Load(ShaderPod* pod, const uri::Uri& path)
{
	return ShaderLoader::Load(pod, path);
}

bool StringPod::Load(StringPod* pod, const uri::Uri& path)
{
	return TextLoader::Load(pod, path);
}

bool TexturePod::Load(TexturePod* pod, const uri::Uri& path)
{
	if (uri::MatchesExtension(path, ".gltf")) {
		return GltfTextureLoader::Load(pod, path);
	}
	if (uri::MatchesExtension(path, ".json")) {
		return CubemapLoader::Load(pod, path);
	}

	return CustomLoader::Load(pod, path);
}

bool JsonDocPod::Load(JsonDocPod* pod, const uri::Uri& path)
{
	return JsonDocLoader::Load(pod, path);
}
