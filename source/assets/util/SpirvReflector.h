#pragma once
#include "assets/shared/ShaderStageShared.h"

struct SpirvReflector {
	static SpirvReflection Reflect(const std::vector<uint32>& code);
};
