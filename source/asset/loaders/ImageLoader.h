#pragma once

#include "asset/pods/ImagePod.h"

#include "stb_image/stb_image.h"
#include "asset/UriLibrary.h"

namespace ImageLoader
{
	inline bool Load(ImagePod* pod, const uri::Uri& path)
	{
		const auto finalPath = uri::ToSystemPath(path);

		pod->isHdr = stbi_is_hdr(finalPath) == 1;

		if (!pod->isHdr)
			pod->data = stbi_load(finalPath, &pod->width, &pod->height, &pod->components, STBI_rgb_alpha);
		else
			pod->data = stbi_loadf(finalPath, &pod->width, &pod->height, &pod->components, STBI_rgb_alpha);

		if (!pod->data || (pod->width == 0) || (pod->height == 0))
		{
			LOG_WARN("TextureAsset loading failed, filepath: {}, data_empty: {} width: {} height: {}", finalPath,
				static_cast<bool>(pod->data), pod->width, pod->height);

			return false;
		}

		return true;

	}
};