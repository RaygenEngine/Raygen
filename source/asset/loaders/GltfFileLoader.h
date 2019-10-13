#pragma once

#include "asset/pods/GltfFilePod.h"
#include "asset/UriLibrary.h"

namespace GltfFileLoader {
inline void Load(GltfFilePod* pod, const uri::Uri& path)
{
	timer::ScopedTimer<ch::milliseconds> timer("Gltf File Asset Load");

	namespace tg = tinygltf;

	tg::TinyGLTF loader;

	std::string err;
	std::string warn;

	const bool ret = loader.LoadASCIIFromFile(&pod->data, &err, &warn, uri::ToSystemPath(uri::GetDiskPath(path)));

	CLOG_WARN(!warn.empty(), "Gltf Load warning for {}: {}", path, warn.c_str());
	CLOG_ABORT(!err.empty(), "Gltf Load error for {}: {}", path, err.c_str());
}
}; // namespace GltfFileLoader
