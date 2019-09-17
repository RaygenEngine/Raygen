#pragma once

#include "system/reflection/Reflector.h"
#include "asset/AssetPod.h"

struct ImagePod : DeletableAssetPod
{
	STATIC_REFLECTOR(ImagePod)
	{
		using namespace PropertyFlags;
		S_REFLECT_VAR(width, NoEdit);
		S_REFLECT_VAR(height, NoEdit);

		S_REFLECT_VAR(components, NoEdit);
		S_REFLECT_VAR(hdr, NoEdit);
	}
	static bool Load(ImagePod* pod, const fs::path& path);

	int32 width{ 0 };
	int32 height{ 0 };
	// actual textures components -> final texture has RGBA, but this value gives insight to how the data is stored
	// however same texture types (e.g heightmaps) may have different actual components (1-grayscale, 4-rgba with r=g=b) based on the image's properties
	// therefore this value isn't very useful 
	int32 components{ 0 };

	// use malloc and free
	void* data{ nullptr };

	// if(isHdr) data -> float* else data -> byte*
	bool isHdr{ false };
};

