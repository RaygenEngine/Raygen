#pragma once

#include "asset/pods/StringPod.h"
#include "asset/UriLibrary.h"

namespace TextLoader {
inline bool Load(StringPod* pod, const uri::Uri& path)
{
	std::ifstream t(uri::ToSystemPath(path));

	if (!t.is_open()) {
		LOG_WARN("Unable to open string file, path: {}", uri::ToSystemPath(path));
		return false;
	}

	t.seekg(0, std::ios::end);
	pod->data.reserve(t.tellg());

	t.seekg(0, std::ios::beg);
	pod->data.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	return true;
}
}; // namespace TextLoader
