#pragma once

#include "asset/pods/GltfFilePod.h"
#include "asset/UriLibrary.h"

namespace GltfFileLoader {
inline bool Load(GltfFilePod* pod, const uri::Uri& path)
{
	Timer::ScopedTimer<ch::milliseconds> timer("Gltf File Asset Load");

	namespace tg = tinygltf;

	tg::TinyGLTF loader;

	std::string err;
	std::string warn;

	const bool ret = loader.LoadASCIIFromFile(&pod->data, &err, &warn, uri::ToSystemPath(uri::GetDiskPath(path)));

	CLOG_WARN(!warn.empty(), warn.c_str());
	CLOG_ERROR(!err.empty(), err.c_str());

	return ret;
}
}; // namespace GltfFileLoader
