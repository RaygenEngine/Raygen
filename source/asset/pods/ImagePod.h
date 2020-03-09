#pragma once

#include "asset/AssetPod.h"
#include "reflection/GenMacros.h"
struct ImagePod : AssetPod {
	REFLECTED_POD(ImagePod)
	{
		using namespace PropertyFlags;
		REFLECT_ICON(FA_IMAGE);

		REFLECT_VAR(width, NoEdit);
		REFLECT_VAR(height, NoEdit);

		REFLECT_VAR(isHdr, NoEdit);
	}
	static void Load(PodEntry* entry, ImagePod* pod, const uri::Uri& path);

	int32 width{ 0 };
	int32 height{ 0 };


	std::vector<byte> data{};

	// DOC: if(isHdr) data -> float* else data -> byte*
	bool isHdr{ false };
};
