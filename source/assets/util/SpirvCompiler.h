#pragma once

#include <map>

struct TextCompilerErrors {
	std::map<int, std::string> errors;
};

struct ShaderCompiler {
	static std::vector<uint32> Compile(
		const std::string& code, const std::string& shadername, TextCompilerErrors* outError = nullptr);
};
