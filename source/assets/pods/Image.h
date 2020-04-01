#pragma once
#include "assets/AssetPod.h"
#include "reflection/GenMacros.h"

enum class ImageFormat
{
	Unorm,
	Srgb,
	Hdr
};

// DOC:
struct Image : AssetPod {

	REFLECTED_POD(Image)
	{
		using namespace PropertyFlags;
		REFLECT_ICON(FA_IMAGE);

		REFLECT_VAR(width, NoEdit);
		REFLECT_VAR(height, NoEdit);

		REFLECT_VAR(format, NoEdit);
	}

	// default imagepod is byte/1x1/white
	int32 width{ 1 };
	int32 height{ 1 };

	std::vector<byte> data{ 0xFF, 0xFF, 0xFF, 0xFF };

	ImageFormat format{ ImageFormat::Unorm };
};
