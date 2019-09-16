#pragma once

#include "asset/pods/GltfFilePod.h"

namespace GltfFileLoader 
{
	inline bool Load(GltfFilePod* pod, const fs::path& path)
	{
		Timer::ScopedTimer<ch::milliseconds> timer("Gltf File Asset Load");

		namespace tg = tinygltf;

		tg::TinyGLTF loader;

		std::string err;
		std::string warn;

		auto ext = path.extension();

		const bool ret = loader.LoadASCIIFromFile(&pod->data, &err, &warn, path.string());

		CLOG_WARN(!warn.empty(), warn.c_str());
		CLOG_ERROR(!err.empty(), err.c_str());

		return ret;
	}
};
