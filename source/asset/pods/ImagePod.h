#pragma once

#include "asset/AssetPod.h"
#include "reflection/GenMacros.h"
struct ImagePod : AssetPod {
	REFLECTED_POD(ImagePod)
	{
		using namespace PropertyFlags;
		REFLECT_VAR(width, NoEdit);
		REFLECT_VAR(height, NoEdit);

		REFLECT_VAR(components, NoEdit);
		REFLECT_VAR(isHdr, NoEdit);
	}
	static void Load(ImagePod* pod, const uri::Uri& path);

	int32 width{ 0 };
	int32 height{ 0 };

	int32 components{ 0 };

	std::vector<byte> data{};

	// if(isHdr) data -> float* else data -> byte*
	bool isHdr{ false };
};
