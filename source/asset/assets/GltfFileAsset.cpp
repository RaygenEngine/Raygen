#include "pch.h"

#include "asset/assets/GltfFileAsset.h"
#include "asset/AssetManager.h"

#include "tinygltf/tiny_gltf.h"
#include <optional>


bool GltfFileAsset::Load(GltfFilePod* pod, const fs::path& path)
{
	Timer::ScopedTimer<ch::milliseconds> timer("Gltf File Asset Load");

	namespace tg = tinygltf;

	tg::TinyGLTF loader;

	std::string err;
	std::string warn;

	auto ext = path.extension();

	bool ret = loader.LoadASCIIFromFile(&pod->data, &err, &warn, path.string());

	CLOG_WARN(!warn.empty(), warn.c_str());
	CLOG_ERROR(!err.empty(), err.c_str());

	return ret;
}
