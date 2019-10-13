#pragma once

#include "asset/AssetPod.h"

#include <nlohmann/json.hpp>

struct JsonDocPod : public AssetPod {
	REFLECTED_POD(JsonDocPod) {}
	static void Load(JsonDocPod* pod, const uri::Uri& path);

	nlohmann::json document;
};
