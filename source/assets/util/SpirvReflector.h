#pragma once
#include <vector>

#include "assets/pods/Shader.h"
#include "assets/pods/Material.h"

struct SpirvReflector {
	static SpirvReflection Reflect(const std::vector<uint32>& code);

	static MaterialParamsArchetype ReflectArchetype(const std::vector<uint32>& code);
};
