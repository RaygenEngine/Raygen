#pragma once
#include "assets/AssetPod.h"

struct Shader : public AssetPod {
	REFLECTED_POD(Shader) { REFLECT_VAR(files); }

	std::vector<int32> files;
};
