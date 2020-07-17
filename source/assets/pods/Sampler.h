#pragma once
#include "assets/AssetPod.h"
#include "reflection/GenMacros.h"


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
