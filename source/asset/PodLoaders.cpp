#include "asset/pods/CubemapPod.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/pods/ImagePod.h"
#include "asset/pods/MaterialPod.h"
#include "asset/pods/ModelPod.h"
#include "asset/pods/ShaderPod.h"
#include "asset/pods/TextPod.h"
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

constexpr char SubpathIdentifier = '#';

bool IsSubpath(const fs::path& path)
{
	return path.filename().string()[0] == SubpathIdentifier;
}

fs::path GetRealExtension(const fs::path& path) 
{
	if (IsSubpath(path)) 
	{
		return path.parent_path().extension();
	}
	return path.extension();
}

bool IsOfType(const fs::path& path, const fs::path& ext)
{
	return GetRealExtension(path) == ext;
}


bool CubemapPod::Load(CubemapPod* pod, const fs::path& path)
{
	return CubemapLoader::Load(pod, path);
}

bool GltfFilePod::Load(GltfFilePod* pod, const fs::path& path)
{
	return GltfFileLoader::Load(pod, path);
}

bool ImagePod::Load(ImagePod* pod, const fs::path& path)
{
	return ImageLoader::Load(pod, path);
}

bool MaterialPod::Load(MaterialPod* pod, const fs::path& path)
{
	if (IsOfType(path, ".gltf")) 
	{
		return GltfMaterialLoader::Load(pod, path);
	}
	// TODO: Json loader

	
	return DefaultMaterialLoader::Load(pod, path);
}

bool ModelPod::Load(ModelPod* pod, const fs::path& path)
{
	if (IsOfType(path, ".gltf")) 
	{
		return GltfModelLoader::Load(pod, path);
	}
	// Add obj loader or others
	
	return false;
}

bool ShaderPod::Load(ShaderPod* pod, const fs::path& path)
{
	return ShaderLoader::Load(pod, path);
}

bool TextPod::Load(TextPod* pod, const fs::path& path)
{
	return TextLoader::Load(pod, path);
}

bool TexturePod::Load(TexturePod* pod, const fs::path& path)
{
	if (IsOfType(path, ".gltf")) 
	{
		return GltfTextureLoader::Load(pod, path);
	}
	// add Json sampler loader

	return DefaultTextureLoader::Load(pod, path);
}

bool XMLDocPod::Load(XMLDocPod* pod, const fs::path& path)
{
	return XMLDocLoader::Load(pod, path);
}