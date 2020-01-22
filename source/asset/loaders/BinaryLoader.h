#pragma once

#include "asset/pods/BinaryPod.h"
#include "asset/UriLibrary.h"

#include <fstream>

// TODO options
namespace BinaryLoader {
inline void Load(BinaryPod* pod, const uri::Uri& path)
{
	std::ifstream t(uri::ToSystemPath(path), std::ios::binary);

	CLOG_ABORT(!t.is_open(), "Unable to open binary file, path: {}", uri::ToSystemPath(path));

	t.seekg(0, std::ios::end);
	pod->data.reserve(t.tellg());

	t.seekg(0, std::ios::beg);
	pod->data.assign((std::istreambuf_iterator<char>(t)), std::istreambuf_iterator<char>());

	t.close();
}
}; // namespace BinaryLoader
