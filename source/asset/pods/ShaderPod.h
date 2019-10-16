#pragma once

#include "asset/AssetPod.h"
#include "asset/pods/StringPod.h"

struct ShaderPod : public AssetPod {
	REFLECTED_POD(ShaderPod) { REFLECT_VAR(files); }
	static void Load(ShaderPod* pod, const uri::Uri& path);

	std::vector<PodHandle<StringPod>> files;
};
