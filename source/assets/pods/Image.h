#pragma once
#include "assets/AssetPod.h"
#include "reflection/GenMacros.h"

// DOC:
struct Image : AssetPod {

	REFLECTED_POD(Image)
	{
		using namespace PropertyFlags;
		REFLECT_ICON(FA_IMAGE);

		REFLECT_VAR(width, NoEdit);
		REFLECT_VAR(height, NoEdit);

		REFLECT_VAR(format);
	}

	// default imagepod is 1mip/byte/1x1/white
	int32 width{ 1 };
	int32 height{ 1 };

	std::vector<byte> data{ 0xFF, 0xFF, 0xFF, 0xFF };

	std::vector<std::vector<byte>> mipData;

	ImageFormat format{ ImageFormat::Unorm };
};
