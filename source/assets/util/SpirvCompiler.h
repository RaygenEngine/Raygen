#pragma once

struct ShaderCompiler {
	static std::vector<uint32> Compile(const std::string& code, const std::string& shadername);
};
