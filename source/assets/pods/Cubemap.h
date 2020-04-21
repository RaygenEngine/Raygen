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

		REFLECT_VAR(width, NoEdit);
		REFLECT_VAR(height, NoEdit);

		REFLECT_VAR(format, NoEdit);
		REFLECT_VAR(faces, NoEdit);
	}

	int32 width{ 1 };
	int32 height{ 1 };

	ImageFormat format{ ImageFormat::Unorm };

	std::vector<PodHandle<::Image>> faces{
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
	};
};
