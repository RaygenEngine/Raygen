#pragma once
#include "assets/PodHandle.h"
#include "assets/pods/ImagePod.h"

enum class TextureFiltering
{
	Nearest,
	Linear,
};

enum class MipmapFiltering
{
	Nearest,
	Linear,
	NoMipmap
};

// CHECK: (mirrored clamping etc)
enum class TextureWrapping
{
	ClampToEdge,
	MirroredRepeat,
	Repeat
};

struct Sampler : AssetPod {
	REFLECTED_POD(Sampler)
	{
		REFLECT_ICON(FA_IMAGES);

		REFLECT_VAR(minFilter);
		REFLECT_VAR(magFilter);

		REFLECT_VAR(mipmapFilter);

		REFLECT_VAR(wrapU);
		REFLECT_VAR(wrapV);
		REFLECT_VAR(wrapW);
	}

	TextureFiltering minFilter{ TextureFiltering::Linear };
	TextureFiltering magFilter{ TextureFiltering::Linear };

	MipmapFiltering mipmapFilter{ MipmapFiltering::Linear };

	TextureWrapping wrapU{ TextureWrapping::Repeat };
	TextureWrapping wrapV{ TextureWrapping::Repeat };
	TextureWrapping wrapW{ TextureWrapping::Repeat };

	// CHECK: anisotropy, mip lod stuff
};
