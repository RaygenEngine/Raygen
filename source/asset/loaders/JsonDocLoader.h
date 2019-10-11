#pragma once

#include "asset/pods/JsonDocPod.h"
#include "asset/UriLibrary.h"
#include <istream>

namespace JsonDocLoader {
inline bool Load(JsonDocPod* pod, const uri::Uri& path)
{
	using json = nlohmann::json;
	std::ifstream file(uri::ToSystemPath(path));

	if (!file.is_open()) {
		LOG_ERROR("Failed to open file at: {}", path);
		return false;
	}
	file >> pod->document;
	return true;
}
}; // namespace JsonDocLoader
