#pragma once
#include "assets/AssetPod.h"
#include "assets/shared/ShaderStageShared.h"

struct ShaderStage : public AssetPod {
	REFLECTED_POD(ShaderStage)
	{
		REFLECT_ICON(FA_CODE);

		REFLECT_VAR(stage);
		REFLECT_VAR(code, PropertyFlags::Multiline);
		REFLECT_VAR(headers /*, PropertyFlags::Advanced*/); // WIP: make this advanced
	}

	// Returns true if compilation was successful
	bool Compile(TextCompilerErrors& outErrors);
	// Compile from specific code. Intented to be used from shader registry, or at most shader editor.
	bool Compile(TextCompilerErrors& outErrors, const std::string& processedCode);
	// Returns true if compilation was successful, errors are reported inline with errorCtxFilename as the filename
	bool CompileInlineErrors(const std::string& errorCtxFilename);

	std::vector<PodHandle<ShaderHeader>> headers;


	ShaderStageType stage{};

	SpirvReflection reflection;

	std::vector<uint32_t> binary;
	std::string code;

private:
	// Returns true on success
	bool CompileInternal(const std::string& errorCtxFilename, TextCompilerErrors* outErrors = nullptr,
		const std::string* processedCode = nullptr);
};
