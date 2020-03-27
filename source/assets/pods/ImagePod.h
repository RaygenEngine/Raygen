#pragma once
#include "assets/AssetPod.h"
#include "reflection/GenMacros.h"

// DOC:
struct ImagePod : AssetPod {
	REFLECTED_POD(ImagePod)
	{
		using namespace PropertyFlags;
		REFLECT_ICON(FA_IMAGE);

		REFLECT_VAR(width, NoEdit);
		REFLECT_VAR(height, NoEdit);

		REFLECT_VAR(isHdr, NoEdit);
	}

	// default imagepod is byte/1x1/white
	int32 width{ 1 };
	int32 height{ 1 };

	std::vector<byte> data{ 0xFF, 0xFF, 0xFF, 0xFF };

	// byte* or float*
	bool isHdr{ false };
};
