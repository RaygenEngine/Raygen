#pragma once
#include <vector>

struct SpirvReflection {
};

struct SpirvReflector {
	static SpirvReflection Reflect(const std::vector<uint32>& code);
};
