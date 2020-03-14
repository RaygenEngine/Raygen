#pragma once

#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"
#include "asset/pods/MaterialPod.h"
#include "asset/pods/ImagePod.h"

#include <nlohmann/json.hpp>

namespace CustomLoader {

inline PodHandle<TexturePod> GetWhiteTexture()
{
	nlohmann::json j;
	j["color"] = "white";
	return AssetImporterManager::OLD_ResolveOrImport<TexturePod>(uri::MakeChildJson("/genTexture", j));
}

inline PodHandle<TexturePod> GetNormalTexture()
{
	nlohmann::json j;
	j["color"] = "normal";
	return AssetImporterManager::OLD_ResolveOrImport<TexturePod>(uri::MakeChildJson("/genTexture", j));
}

inline PodHandle<MaterialPod> GetDefaultMat()
{
	return AssetImporterManager::OLD_ResolveOrImport<MaterialPod>("/defaultMat");
}

inline void AllocateImage(ImagePod* pod, glm::u8vec4 color)
{
	pod->width = 1;
	pod->height = 1;

	pod->isHdr = false;

	pod->data.resize(sizeof(glm::u8vec4));

	byte* d = pod->data.data();
	d[0] = color.r;
	d[1] = color.g;
	d[2] = color.b;
	d[3] = color.a;
}

inline void Load(PodEntry* entry, ImagePod* pod, const uri::Uri& path)
{
	nlohmann::json j = uri::GetJson(path);

	auto str = j.value<std::string>("color", "");

	if (str == "normal") {
		AllocateImage(pod, glm::u8vec4(128, 127, 255, 255));
		return;
	}

	AllocateImage(pod, glm::u8vec4(255));
}

inline void Load(PodEntry* entry, TexturePod* pod, const uri::Uri& path)
{
	nlohmann::json j = uri::GetJson(path);
	pod->image = AssetImporterManager::OLD_ResolveOrImport<ImagePod>(uri::MakeChildJson("/genImg", j));
}

inline void Load(PodEntry* entry, MaterialPod* pod, const uri::Uri&)
{
	pod->baseColorTexture = GetWhiteTexture();
	pod->normalTexture = GetNormalTexture();
	pod->emissiveTexture = GetWhiteTexture();
	pod->metallicRoughnessTexture = GetWhiteTexture();
	pod->occlusionTexture = GetWhiteTexture();
}


} // namespace CustomLoader
