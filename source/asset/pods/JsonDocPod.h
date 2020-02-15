#pragma once

#include "asset/AssetPod.h"
#include "reflection/GenMacros.h"
#include <nlohmann/json.hpp>

struct JsonDocPod : public AssetPod {
	REFLECTED_POD(JsonDocPod) {}
	static void Load(PodEntry* entry, JsonDocPod* pod, const uri::Uri& path);

	nlohmann::json document;
};
