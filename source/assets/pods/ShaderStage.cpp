#include "ShaderStage.h"

#include "assets/util/SpirvCompiler.h"


bool ShaderStage::Compile(TextCompilerErrors& outErrors)
{
	return CompileInternal("", &outErrors);
}

bool ShaderStage::Compile(TextCompilerErrors& outErrors, const std::string& processedCode)
{
	return CompileInternal("", nullptr, &processedCode);
}

bool ShaderStage::CompileInlineErrors(const std::string& errorCtxFilename)
{
	return CompileInternal(errorCtxFilename, nullptr);
}

bool ShaderStage::CompileInternal(
	const std::string& errorCtxFilename, TextCompilerErrors* outErrors, const std::string* processedCode)
{
	auto bincode = ShaderCompiler::Compile(processedCode ? *processedCode : code, stage, errorCtxFilename, outErrors);
	if (!bincode.empty()) {
		binary.swap(bincode);
		return true;
	}
	return false;
}
