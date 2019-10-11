#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"

#include <fstream>

namespace CubemapLoader {
inline bool Load(TexturePod* pod, const uri::Uri& path)
{
	std::ifstream inStream(uri::ToSystemPath(path));

	if (!inStream.is_open()) {
		LOG_WARN("Unable to open string file, path: {}", uri::ToSystemPath(path));
		return false;
	}
	int32 firstChar = inStream.peek();

	if (firstChar == EOF) {
		LOG_WARN("Found empty cubemap file: {} No images loaded", uri::ToSystemPath(path));
		return false;
	}

	pod->target = TextureTarget::TEXTURE_CUBEMAP;
	pod->images.resize(CubemapFace::COUNT);

	nlohmann::json j;
	inStream >> j;

	std::string newFilePath;

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
			pod->images[value] = AssetManager::GetOrCreateFromParentUri<ImagePod>(imagePath, path);
		}
		else {
			LOG_ERROR("Missing cubemap face: \"{}\" when loading: {}", key, path);
		}
	}

	return true;
}
}; // namespace CubemapLoader
