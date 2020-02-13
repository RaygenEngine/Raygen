#pragma once

#include "asset/pods/ImagePod.h"
#include "asset/UriLibrary.h"

#include <stb_image.h>

namespace ImageLoader {
inline void Load(ImagePod* pod, const uri::Uri& path)
{
	const auto finalPath = uri::ToSystemPath(path);

	pod->isHdr = stbi_is_hdr(finalPath) == 1;

	void* data = nullptr;
	if (!pod->isHdr) {
		data = stbi_load(finalPath, &pod->width, &pod->height, &pod->components, STBI_default);
	}
	else {
		data = stbi_loadf(finalPath, &pod->width, &pod->height, &pod->components, STBI_default);
	}

	const bool hasNotResult = !data || (pod->width == 0) || (pod->height == 0);

	CLOG_ABORT(hasNotResult, "TexturePod loading failed, filepath: {}, data_empty: {} width: {} height: {}", finalPath,
		static_cast<bool>(data), pod->width, pod->height);

	// PERF: crappy std::vector initialization on resize,
	// to solve fork stb_image and pass preallocated pointer to loading functions
	size_t byteCount = (pod->width * pod->height * pod->components) * (pod->isHdr ? sizeof(float) : sizeof(byte));
	pod->data.resize(byteCount);

	memcpy(pod->data.data(), data, byteCount);
	free(data);
}
}; // namespace ImageLoader
