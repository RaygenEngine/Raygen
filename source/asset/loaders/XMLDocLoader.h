#pragma once

#include "asset/pods/XMLDocPod.h"
#include "asset/UriLibrary.h"

namespace JsonDocLoader {
inline bool Load(JsonDocPod* pod, const uri::Uri& path)
{
	using nlohmann::json;
	std::ifstream file(uri::ToSystemPath(path));

	if (!file.is_open()) {
		LOG_ERROR("Failed to open file at: {}", path);
		return false;
	}
	file >> pod->document;
	return true;
}
}; // namespace JsonDocLoader
