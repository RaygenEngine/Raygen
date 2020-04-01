#include "pch.h"
#include "ImageImporter.h"

#include "assets/AssetImporterManager.h"

#include <stb/stb_image.h>

BasePodHandle ImageImporter::Import(const fs::path& path)
{
	const auto finalPath = path.generic_string();

	auto& [handle, pod]
		= ImporterManager->CreateEntry<Image>(path.generic_string(), path.filename().replace_extension().string());

	auto isHdr = stbi_is_hdr(finalPath.c_str()) == 1;

	void* data = nullptr;
	if (!isHdr) {
		data = stbi_load(finalPath.c_str(), &pod->width, &pod->height, nullptr, STBI_rgb_alpha);
	}
	else {
		pod->format = ImageFormat::Hdr;
		data = stbi_loadf(finalPath.c_str(), &pod->width, &pod->height, nullptr, STBI_rgb_alpha);
	}

	const bool hasNotResult = !data || (pod->width == 0) || (pod->height == 0);

	CLOG_ABORT(hasNotResult, "Image loading failed, filepath: {}, data_empty: {} width: {} height: {}", finalPath,
		static_cast<bool>(data), pod->width, pod->height);

	// PERF: crappy std::vector initialization on resize,
	// to solve fork stb_image and pass preallocated pointer to loading functions
	size_t byteCount = (pod->width * pod->height * 4llu) * (isHdr ? sizeof(float) : sizeof(byte));
	pod->data.resize(byteCount);

	memcpy(pod->data.data(), data, byteCount);
	free(data);

	return handle;
}
