#pragma once

#include "asset/pods/StringPod.h"

struct ShaderCompiler {
	static std::vector<uint32> Compile(const std::string& filename);
};
