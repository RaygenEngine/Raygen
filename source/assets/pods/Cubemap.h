#pragma once
#include "assets/AssetPod.h"
#include "assets/PodHandle.h"
#include "assets/pods/Image.h"
#include "reflection/GenMacros.h"

#include <array>


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

		REFLECT_VAR(format, NoEdit);
		REFLECT_VAR(faces, NoEdit);

		REFLECT_VAR(irradiance, NoEdit);
	}

	int32 resolution{ 1 };

	ImageFormat format{ ImageFormat::Unorm };

	std::vector<PodHandle<::Image>> faces{
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
	};

	std::vector<PodHandle<::Image>> irradiance{
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
		PodHandle<::Image>(),
	};
};
