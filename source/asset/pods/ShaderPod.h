#pragma once

#include "asset/AssetPod.h"

struct ShaderPod : public AssetPod {
	REFLECTED_POD(ShaderPod) { REFLECT_VAR(files); }
	static void Load(PodEntry* entry, ShaderPod* pod, const uri::Uri& path);

	std::vector<int32> files;
};
