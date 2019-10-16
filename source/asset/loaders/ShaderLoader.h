#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/ShaderPod.h"
#include "asset/UriLibrary.h"

// if found same included file then abort
#include <unordered_set>
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

	for (auto newFilePath : j) {
		pod->files.emplace_back(
			AssetManager::GetOrCreateFromParentUri<StringPod>(newFilePath.get<std::string>(), path));
	}
}


}; // namespace ShaderLoader
