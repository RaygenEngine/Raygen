#pragma once
#include "assets/AssetPod.h"
#include "assets/PodHandle.h"
#include "assets/pods/Image.h"
#include "reflection/GenMacros.h"

#include <array>

enum CubemapFace : int32
{
	Up = 0,
	Down = 1,
	Right = 2,
	Left = 3,
	Front = 4,
	Back = 5
};

// DOC: currently Hdr only for IBL support, TODO: split
struct Cubemap : AssetPod {

	REFLECTED_POD(Cubemap)
	{
		using namespace PropertyFlags;
		REFLECT_ICON(FA_CUBE);

		REFLECT_VAR(width, NoEdit);
		REFLECT_VAR(height, NoEdit);

		REFLECT_VAR(format, NoEdit);
		REFLECT_VAR(faces, NoEdit);
	}

	int32 width{ 0 };
	int32 height{ 0 };

	ImageFormat format{ ImageFormat::Unorm };

	std::vector<PodHandle<::Image>> faces;
};
