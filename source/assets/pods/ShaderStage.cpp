#include "ShaderStage.h"

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
	return false; // NEW:
}
