#pragma once

#include "core/reflection/GenMacros.h"
#include "asset/AssetPod.h"

#include "nlohmann/json.hpp"

struct JsonDocPod : public AssetPod {
	REFLECTED_POD(JsonDocPod) {}
	static bool Load(JsonDocPod* pod, const uri::Uri& path);

	nlohmann::json document;
};
