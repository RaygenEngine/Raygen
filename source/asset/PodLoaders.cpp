#include "asset/pods/CubemapPod.h"
#include "asset/pods/GltfFilePod.h"
#include "asset/pods/ImagePod.h"
#include "asset/pods/MaterialPod.h"
#include "asset/pods/ModelPod.h"
#include "asset/pods/ShaderPod.h"
#include "asset/pods/TextPod.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/XMLDocPod.h"

#include "asset/assets/CubemapAsset.h"
#include "asset/assets/DummyAssets.h"
#include "asset/assets/GltfFileAsset.h"
#include "asset/assets/GltfMaterialAsset.h"
#include "asset/assets/GltfModelAsset.h"
#include "asset/assets/GltfTextureAsset.h"
#include "asset/assets/ImageAsset.h"
#include "asset/assets/ShaderAsset.h"
#include "asset/assets/TextAsset.h"
#include "asset/assets/XMLDocAsset.h"

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
	return CubemapAsset::Load(pod, path);
}

bool GltfFilePod::Load(GltfFilePod* pod, const fs::path& path)
{
	return GltfFileAsset::Load(pod, path);
}

bool ImagePod::Load(ImagePod* pod, const fs::path& path)
{
	return ImageAsset::Load(pod, path);
}

bool MaterialPod::Load(MaterialPod* pod, const fs::path& path)
{
	if (IsOfType(path, ".gltf")) 
	{
		return GltfMaterialAsset::Load(pod, path);
	}
	// TODO: Json loader

	
	return DefaultMaterial::Load(pod, path);
}

bool ModelPod::Load(ModelPod* pod, const fs::path& path)
{
	if (IsOfType(path, ".gltf")) 
	{
		return GltfModelAsset::Load(pod, path);
	}
	// Add obj loader or others
	
	return false;
}

bool ShaderPod::Load(ShaderPod* pod, const fs::path& path)
{
	return ShaderAsset::Load(pod, path);
}

bool TextPod::Load(TextPod* pod, const fs::path& path)
{
	return TextAsset::Load(pod, path);
}

bool TexturePod::Load(TexturePod* pod, const fs::path& path)
{
	if (IsOfType(path, ".gltf")) 
	{
		return GltfTextureAsset::Load(pod, path);
	}
	// add Json sampler loader

	return DefaultTexture::Load(pod, path);
}

bool XMLDocPod::Load(XMLDocPod* pod, const fs::path& path)
{
	return XMLDocAsset::Load(pod, path);
}