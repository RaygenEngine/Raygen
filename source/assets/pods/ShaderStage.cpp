#include "ShaderStage.h"

#include "assets/util/SpirvCompiler.h"


bool ShaderStage::Compile(TextCompilerErrors& outErrors)
{
	return Compile("", &outErrors);
}

bool ShaderStage::CompileInlineErrors(const std::string& errorCtxFilename)
{
	return Compile(errorCtxFilename, nullptr);
}

bool ShaderStage::Compile(const std::string& errorCtxFilename, TextCompilerErrors* outErrors)
{
	auto bincode = ShaderCompiler::Compile(code, stage, errorCtxFilename, outErrors);
	if (!bincode.empty()) {
		binary.swap(bincode);
		return true;
	}
	return false;
}
