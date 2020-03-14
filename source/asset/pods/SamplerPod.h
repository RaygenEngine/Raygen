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

// CHECK: (mirrored clamping etc)
enum class TextureWrapping
{
	ClampToEdge,
	MirroredRepeat,
	Repeat
};

struct SamplerPod : AssetPod {
	REFLECTED_POD(SamplerPod)
	{
		REFLECT_ICON(FA_IMAGES);

		REFLECT_VAR(minFilter);
		REFLECT_VAR(magFilter);

		REFLECT_VAR(wrapU);
		REFLECT_VAR(wrapV);
		REFLECT_VAR(wrapW);
	}
	static void Load(PodEntry* entry, SamplerPod* pod, const uri::Uri& path);

	TextureFiltering minFilter{ TextureFiltering::Linear };
	TextureFiltering magFilter{ TextureFiltering::Linear };

	TextureWrapping wrapU{ TextureWrapping::Repeat };
	TextureWrapping wrapV{ TextureWrapping::Repeat };
	TextureWrapping wrapW{ TextureWrapping::Repeat };
};
