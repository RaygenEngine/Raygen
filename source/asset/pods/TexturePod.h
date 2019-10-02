#pragma once

#include "core/reflection/GenMacros.h"
#include "asset/AssetPod.h"
#include "asset/pods/ImagePod.h"

struct TexturePod : public AssetPod
{
	REFLECTED_POD(TexturePod)
	{
		REFLECT_VAR(images);
	}
	static bool Load(TexturePod* pod, const fs::path& path);

	TextureFiltering minFilter{ TextureFiltering::LINEAR };
	TextureFiltering magFilter{ TextureFiltering::LINEAR };

	TextureWrapping wrapS{ TextureWrapping::REPEAT };
	TextureWrapping wrapT{ TextureWrapping::REPEAT };
	TextureWrapping wrapR{ TextureWrapping::REPEAT };

	TextureTarget target{ TextureTarget::TEXTURE_2D };
	
	std::vector<PodHandle<ImagePod>> images;
};

