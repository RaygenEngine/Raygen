#include "pch/pch.h"

#include "asset/pods/GltfFilePod.h"
#include "asset/pods/ImagePod.h"
#include "asset/pods/MaterialPod.h"
#include "asset/pods/ModelPod.h"
#include "asset/pods/ShaderPod.h"
#include "asset/pods/StringPod.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/XMLDocPod.h"

#include "asset/loaders/CubemapLoader.h"
#include "asset/loaders/DummyLoader.h"
#include "asset/loaders/GltfFileLoader.h"
#include "asset/loaders/GltfMaterialLoader.h"
#include "asset/loaders/GltfModelLoader.h"
#include "asset/loaders/GltfTextureLoader.h"
#include "asset/loaders/ImageLoader.h"
#include "asset/loaders/ShaderLoader.h"
#include "asset/loaders/TextLoader.h"
#include "asset/loaders/XMLDocLoader.h"

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

bool XMLDocPod::Load(XMLDocPod* pod, const uri::Uri& path)
{
	return XMLDocLoader::Load(pod, path);
}
