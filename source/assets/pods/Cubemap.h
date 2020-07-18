#pragma once
#include "assets/AssetPod.h"
#include "assets/shared/ImageShared.h"
#include "reflection/GenMacros.h"

// DOC: resolution = width = height of each face, same goes for format
struct Cubemap : AssetPod {

	enum : int32
	{
		Right = 0,
		Left = 1,
		Up = 2,
		Down = 3,
		Front = 4,
		Back = 5
	};

	REFLECTED_POD(Cubemap)
	{
		using namespace PropertyFlags;
		REFLECT_ICON(FA_CUBE);

		REFLECT_VAR(resolution, NoEdit);
		REFLECT_VAR(mipCount, NoEdit);

		REFLECT_VAR(format);
	}

	// default cubemappod is 1x1/1mip/srgb-byte/white
	int32 resolution{ 1 }; // width = height = resolution
	int32 mipCount{ 1 };

	ImageFormat format{ ImageFormat::Srgb };
	std::vector<byte> data{
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
		0xFF,
	};
};
