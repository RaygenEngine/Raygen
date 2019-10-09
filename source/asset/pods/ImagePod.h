#pragma once

#include "core/reflection/GenMacros.h"
#include "asset/AssetPod.h"

struct ImagePod : AssetPod {
	REFLECTED_POD(ImagePod)
	{
		using namespace PropertyFlags;
		REFLECT_VAR(width, NoEdit);
		REFLECT_VAR(height, NoEdit);

		REFLECT_VAR(components, NoEdit);
		REFLECT_VAR(isHdr, NoEdit);
	}
	static bool Load(ImagePod* pod, const uri::Uri& path);

	int32 width{ 0 };
	int32 height{ 0 };
	// actual textures components -> final texture has RGBA, but this value gives insight to how the data is stored
	// however same texture types (e.g heightmaps) may have different actual components (1-grayscale, 4-rgba with r=g=b)
	// based on the image's properties therefore this value isn't very useful
	int32 components{ 0 };

	// use malloc and free
	void* data{ nullptr };

	// if(isHdr) data -> float* else data -> byte*
	bool isHdr{ false };

	~ImagePod() { free(data); }
	ImagePod(const ImagePod&) = default;
	ImagePod(ImagePod&&) = default;
	ImagePod& operator=(const ImagePod&) = default;
	ImagePod& operator=(ImagePod&&) = default;
};
