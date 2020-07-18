#pragma once
#include "assets/AssetPod.h"
#include "assets/shared/ImageShared.h"

// DOC:
struct Image : AssetPod {

	REFLECTED_POD(Image)
	{
		using namespace PropertyFlags;
		REFLECT_ICON(FA_IMAGE);

		REFLECT_VAR(width, NoEdit);
		REFLECT_VAR(height, NoEdit);

		REFLECT_VAR(mipCount, NoEdit);

		REFLECT_VAR(format);
	}

	// default imagepod is 1x1/1mip/unorm-byte/white
	int32 width{ 1 };
	int32 height{ 1 };

	int32 mipCount{ 1 };

	ImageFormat format{ ImageFormat::Unorm };

	// all mip data
	std::vector<byte> data{ 0xFF, 0xFF, 0xFF, 0xFF };

	// 2x2 Bilinear Sample of a non Hdr image (with uv clamping to edge).
	[[nodiscard]] glm::vec4 BilinearSample(float u, float v, glm::vec2 pixelOffset = { 0.0f, 0.0f }) const;
};
