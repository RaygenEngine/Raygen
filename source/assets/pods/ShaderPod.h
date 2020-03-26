#pragma once
#include "assets/AssetPod.h"

struct ShaderPod : public AssetPod {
	REFLECTED_POD(ShaderPod) { REFLECT_VAR(files); }

	std::vector<int32> files;
};
