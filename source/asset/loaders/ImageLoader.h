#pragma once

#include "asset/pods/ImagePod.h"
#include "asset/UriLibrary.h"

#include <stb_image/stb_image.h>

namespace ImageLoader {
inline bool Load(ImagePod* pod, const uri::Uri& path)
{
	const auto finalPath = uri::ToSystemPath(path);

	pod->isHdr = stbi_is_hdr(finalPath) == 1;

	if (!pod->isHdr) {
		pod->data = stbi_load(finalPath, &pod->width, &pod->height, &pod->components, STBI_rgb_alpha);
	}
	else {
		pod->data = stbi_loadf(finalPath, &pod->width, &pod->height, &pod->components, STBI_rgb_alpha);
	}

	bool hasNotResult = !pod->data || (pod->width == 0) || (pod->height == 0);

	CLOG_WARN(hasNotResult, "TexturePod loading failed, filepath: {}, data_empty: {} width: {} height: {}", finalPath,
		static_cast<bool>(pod->data), pod->width, pod->height);

	return !hasNotResult;
}
}; // namespace ImageLoader
