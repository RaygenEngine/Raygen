#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"
#include "asset/pods/ImagePod.h"

struct TexturePod : AssetPod
{
	STATIC_REFLECTOR(TexturePod)
	{
		S_REFLECT_VAR(image);
	}
	static bool Load(TexturePod* pod, const fs::path& path);

	TextureFiltering minFilter{TextureFiltering::LINEAR};
	TextureFiltering magFilter{TextureFiltering::LINEAR};

	TextureWrapping wrapS{TextureWrapping::REPEAT};
	TextureWrapping wrapT{TextureWrapping::REPEAT};
	TextureWrapping wrapR{TextureWrapping::REPEAT};

	PodHandle<ImagePod> image;
};

