#pragma once
#include "assets/AssetPod.h"
#include "assets/PodHandle.h"
#include "assets/pods/Image.h"
#include "reflection/GenMacros.h"

#include <array>


// DOC: currently Hdr only for IBL support, TODO: split
struct Cubemap : AssetPod {

	enum : int32
	{
		Front = 0,
		Back = 1,
		Up = 2,
		Down = 3,
		Right = 4,
		Left = 5,
	};

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
