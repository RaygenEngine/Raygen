#pragma once

#include "asset/PodHandle.h"
#include "asset/pods/ImagePod.h"

// TODO: Pascal Case
enum class TextureFiltering
{
	NEAREST,
	LINEAR,
	NEAREST_MIPMAP_NEAREST,
	NEAREST_MIPMAP_LINEAR,
	LINEAR_MIPMAP_NEAREST,
	LINEAR_MIPMAP_LINEAR
};

enum class TextureWrapping
{
	CLAMP_TO_EDGE,
	MIRRORED_REPEAT,
	REPEAT
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

	TextureFiltering minFilter{ TextureFiltering::LINEAR };
	TextureFiltering magFilter{ TextureFiltering::LINEAR };

	TextureWrapping wrapU{ TextureWrapping::REPEAT };
	TextureWrapping wrapV{ TextureWrapping::REPEAT };
	TextureWrapping wrapW{ TextureWrapping::REPEAT };
};
