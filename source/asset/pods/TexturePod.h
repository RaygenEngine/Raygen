#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/pods/ImagePod.h"

struct TexturePod : AssetPod
{
	static bool Load(TexturePod* pod, const fs::path& path);

	TextureFiltering minFilter{ TextureFiltering::LINEAR };
	TextureFiltering magFilter{ TextureFiltering::LINEAR };

	TextureWrapping wrapS{ TextureWrapping::REPEAT };
	TextureWrapping wrapT{ TextureWrapping::REPEAT };
	TextureWrapping wrapR{ TextureWrapping::REPEAT };

	TextureType type{ TextureType::TEXTURE_2D };
	
	std::vector<PodHandle<ImagePod>> images;
};

