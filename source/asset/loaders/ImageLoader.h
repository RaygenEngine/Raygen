#pragma once

#include "asset/pods/ImagePod.h"
#include "asset/UriLibrary.h"

#include <stb_image.h>

namespace ImageLoader {
inline void Load(ImagePod* pod, const uri::Uri& path)
{
	const auto finalPath = uri::ToSystemPath(path);

	pod->isHdr = stbi_is_hdr(finalPath) == 1;

	if (!pod->isHdr) {
		pod->data = stbi_load(finalPath, &pod->width, &pod->height, &pod->components, STBI_default);
	}
	else {
		pod->data = stbi_loadf(finalPath, &pod->width, &pod->height, &pod->components, STBI_default);
	}

	const bool hasNotResult = !pod->data || (pod->width == 0) || (pod->height == 0);

	CLOG_ABORT(hasNotResult, "TexturePod loading failed, filepath: {}, data_empty: {} width: {} height: {}", finalPath,
		static_cast<bool>(pod->data), pod->width, pod->height);
}
}; // namespace ImageLoader
