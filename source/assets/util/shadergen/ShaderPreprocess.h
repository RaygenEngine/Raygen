#pragma once

#include "assets/ShaderRegistry.h"

namespace shd {
std::vector<std::pair<int, std::string>> ExtractIncludes(const std::string& str);

// Calls directly into ShaderRegistry by itself
std::string PreprocessCode(
	const std::string& code, std::vector<std::tuple<int32, std::string, ShaderRegistry::KNode*>>& outIncludesReplaced);
} // namespace shd
