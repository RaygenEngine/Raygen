#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/ShaderPod.h"
#include "asset/UriLibrary.h"

#include <nlohmann/json.hpp>
#include <fstream>

namespace ShaderLoader {
inline void Load(ShaderPod* pod, const uri::Uri& path)
{
	std::ifstream inStream(uri::ToSystemPath(path));


	CLOG_ABORT(!inStream.is_open(), "Unable to open shader file, path: {}", uri::ToSystemPath(path));

	inStream >> std::ws;

	int32 firstChar = inStream.peek();

	CLOG_ABORT(firstChar == EOF, "Found empty shader file: {} No shaders loaded", uri::ToSystemPath(path));

	using nlohmann::json;
	json j;

	inStream >> j;

	std::string newFilePath;

	newFilePath = j.value<std::string>("vert", "");
	if (!newFilePath.empty()) {
		pod->vertex = AssetManager::GetOrCreateFromParentUri<StringPod>(newFilePath, path);
	}

	newFilePath = j.value<std::string>("geom", "");
	if (!newFilePath.empty()) {
		pod->geometry = AssetManager::GetOrCreateFromParentUri<StringPod>(newFilePath, path);
	}

	newFilePath = j.value<std::string>("frag", "");
	if (!newFilePath.empty()) {
		pod->fragment = AssetManager::GetOrCreateFromParentUri<StringPod>(newFilePath, path);
	}
}

}; // namespace ShaderLoader
