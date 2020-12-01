#pragma once
#include "assets/shared/ShaderStageShared.h"

extern "C" {
// Warning: unsafe usage of containers in dll boundary, assumes we always compile this dll ourselves. Containers also
// included in TextCompilerErrors

void CompileGlslImpl(LogTransactions* log, std::vector<uint32>* outCode, const std::string* inCode,
	const std::string* shaderNamePtr, TextCompilerErrors* outError, ShaderStageType stage);
}
