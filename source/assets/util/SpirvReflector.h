#pragma once
#include <vector>

#include "assets/pods/ShaderPod.h"

struct SpirvReflector {
	static SpirvReflection Reflect(const std::vector<uint32>& code);
};
