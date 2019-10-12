#pragma once

#include "asset/pods/StringPod.h"
#include "asset/UriLibrary.h"

#include <fstream>

namespace TextLoader {
inline void Load(StringPod* pod, const uri::Uri& path)
{
	std::ifstream t(uri::ToSystemPath(path));

	CLOG_ABORT(!t.is_open(), "Unable to open string file, path: {}", uri::ToSystemPath(path));


	t.seekg(0, std::ios::end);
	pod->data.reserve(t.tellg());

	t.seekg(0, std::ios::beg);
	pod->data.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());
}
}; // namespace TextLoader
