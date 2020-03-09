#pragma once

#include "asset/PodHandle.h"
#include "asset/pods/ImagePod.h"

enum class TextureFiltering
{
	Nearest,
	Linear,
	NearestMipmapNearest,
	NearestMipmapLinear,
	LinearMipmapNearest,
	LinearMipmapLinear
};

enum class TextureWrapping
{
	ClampToEdge,
	MirroredRepeat,
	Repeat
};

struct TexturePod : AssetPod {
	REFLECTED_POD(TexturePod)
	{
		REFLECT_ICON(FA_IMAGES);

		REFLECT_VAR(image);

		REFLECT_VAR(minFilter);
		REFLECT_VAR(magFilter);

		REFLECT_VAR(wrapU);
		REFLECT_VAR(wrapV);
		REFLECT_VAR(wrapW);
	}
	static void Load(PodEntry* entry, TexturePod* pod, const uri::Uri& path);

	PodHandle<ImagePod> image{};

	TextureFiltering minFilter{ TextureFiltering::Linear };
	TextureFiltering magFilter{ TextureFiltering::Linear };

	TextureWrapping wrapU{ TextureWrapping::Repeat };
	TextureWrapping wrapV{ TextureWrapping::Repeat };
	TextureWrapping wrapW{ TextureWrapping::Repeat };
};
