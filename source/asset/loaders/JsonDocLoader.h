#pragma once

#include "asset/pods/JsonDocPod.h"
#include "asset/UriLibrary.h"
#include <istream>

namespace JsonDocLoader {
inline void Load(JsonDocPod* pod, const uri::Uri& path)
{
	std::ifstream file(uri::ToSystemPath(path));

	CLOG_ABORT(!file.is_open(), "Failed to open file at: {}", path);

	file >> pod->document;
}
}; // namespace JsonDocLoader
