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
	return AssetManager::GetOrCreate<TexturePod>(uri::MakeChildJson("/genTexture", j));
}

inline PodHandle<TexturePod> GetNormalTexture()
{
	nlohmann::json j;
	j["color"] = "normal";
	return AssetManager::GetOrCreate<TexturePod>(uri::MakeChildJson("/genTexture", j));
}

inline PodHandle<MaterialPod> GetDefaultMat()
{
	return AssetManager::GetOrCreate<MaterialPod>("/defaultMat");
}

inline void AllocateImage(ImagePod* pod, glm::u8vec4 color)
{
	pod->width = 1;
	pod->height = 1;

	pod->isHdr = false;
	pod->components = 4;

	pod->data = malloc(sizeof(glm::u8vec4));

	byte* d = reinterpret_cast<byte*>(pod->data);
	d[0] = color.r;
	d[1] = color.g;
	d[2] = color.b;
	d[3] = color.a;
}

inline bool Load(ImagePod* pod, const uri::Uri& path)
{
	nlohmann::json j = uri::GetJson(path);

	auto str = j.value<std::string>("color", "");

	if (str.empty() || str == "white") {
		AllocateImage(pod, glm::u8vec4(255));
		return true;
	}

	if (str == "normal") {
		AllocateImage(pod, glm::u8vec4(128, 127, 255, 255));
		return true;
	}

	return false;
}

inline bool Load(TexturePod* pod, const uri::Uri& path)
{
	nlohmann::json j = uri::GetJson(path);
	pod->images.push_back(AssetManager::GetOrCreate<ImagePod>(uri::MakeChildJson("/genImg", j)));
	return true;
}

inline bool Load(MaterialPod* pod, const uri::Uri&)
{
	pod->baseColorTexture = GetWhiteTexture();
	pod->normalTexture = GetNormalTexture();
	pod->emissiveTexture = GetWhiteTexture();
	pod->metallicRoughnessTexture = GetWhiteTexture();
	pod->occlusionTexture = GetWhiteTexture();
	return true;
}


} // namespace CustomLoader
