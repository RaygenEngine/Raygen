#pragma once
#include "assets/util/ShaderStageEnums.h"

struct SpirvReflector {
	static SpirvReflection Reflect(const std::vector<uint32>& code);
};
