#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/ShaderPod.h"
#include "asset/UriLibrary.h"

#include "nlohmann/json.hpp"

namespace ShaderLoader {
inline bool Load(ShaderPod* pod, const uri::Uri& path)
{
	std::ifstream inStream(uri::ToSystemPath(path));

	if (!inStream.is_open()) {
		LOG_WARN("Unable to open shader file, path: {}", uri::ToSystemPath(path));
		return false;
	}

	inStream >> std::ws;

	int32 firstChar = inStream.peek();

	if (firstChar == EOF) {
		LOG_WARN("Found empty shader file: {} No shaders loaded", uri::ToSystemPath(path));
		return false;
	}

	if (firstChar != '{') {
		LOG_REPORT("DEPRECATED text shader file: {}", path);
		char buf[256];

		inStream.getline(buf, 256);
		pod->vertex = AssetManager::GetOrCreateFromParentUri<StringPod>(buf, path);

		inStream.getline(buf, 256);
		pod->fragment = AssetManager::GetOrCreateFromParentUri<StringPod>(buf, path);
		return true;
	}

	using nlohmann::json;
	json j;

	inStream >> j;

	std::string newFilePath;

	newFilePath = j.value<std::string>("vert", "");
	if (!newFilePath.empty()) {
		pod->vertex = AssetManager::GetOrCreateFromParentUri<StringPod>(newFilePath, path);
	}

	newFilePath = j.value<std::string>("frag", "");
	if (!newFilePath.empty()) {
		pod->fragment = AssetManager::GetOrCreateFromParentUri<StringPod>(newFilePath, path);
	}

	return true;
}

}; // namespace ShaderLoader
