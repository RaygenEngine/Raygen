#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"

#include <fstream>

namespace CubemapLoader {
// Returns false if failed to detect its custom json format, allows the generic loader to run if this fails
inline bool Load(TexturePod* pod, const uri::Uri& path)
{
	std::ifstream inStream(uri::ToSystemPath(path));

	CLOG_ABORT(!inStream.is_open(), "Unable to open string file, path: {}", uri::ToSystemPath(path));
	int32 firstChar = inStream.peek();

	CLOG_ABORT(firstChar == EOF, "Found empty cubemap file: {} No images loaded", uri::ToSystemPath(path));

	nlohmann::json j;
	inStream >> j;

	std::string newFilePath;

	bool hasFoundAnyFace = false;

	static std::unordered_map<std::string, int32> imageNames = {
		{ "up", CubemapFace::UP },
		{ "down", CubemapFace::DOWN },
		{ "right", CubemapFace::RIGHT },
		{ "left", CubemapFace::LEFT },
		{ "front", CubemapFace::FRONT },
		{ "back", CubemapFace::BACK },
	};

	for (auto& [key, value] : imageNames) {
		std::string imagePath = j.value(key, "");
		if (!imagePath.empty()) {
			if (!hasFoundAnyFace) {
				pod->target = TextureTarget::TEXTURE_CUBEMAP;
				pod->images.resize(CubemapFace::COUNT);
			}

			pod->images[value] = AssetManager::GetOrCreateFromParentUri<ImagePod>(imagePath, path);
			hasFoundAnyFace = true;
		}
		else if (hasFoundAnyFace) {
			LOG_ERROR("Missing cubemap face: \"{}\" when loading: {}", key, path);
		}
	}

	return hasFoundAnyFace;
}
}; // namespace CubemapLoader
