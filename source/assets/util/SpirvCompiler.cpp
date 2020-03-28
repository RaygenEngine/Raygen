#include "pch.h"
#include "SpirvCompiler.h"

#include "engine/Logger.h"

#include <glslang/OSDependent/osinclude.h>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <StandAlone/ResourceLimits.h>

EShLanguage FindLanguage(const std::string& filename);

std::vector<uint32> ShaderCompiler::Compile(const std::string& filename)
{
	using namespace glslang;

	std::vector<uint32> outCode;

	static bool HasInitGlslLang = false;
	if (!HasInitGlslLang) {
		glslang::InitializeProcess();
		HasInitGlslLang = true;
	}

	std::ifstream file(filename);
	if (!file.is_open()) {
		auto er = fmt::format("Failed to open shader file for compilation: [{}]\n", filename);
		// AppendErrors += er;
		LOG_WARN("{}", er);
		return {};
	}


	std::string InputGLSL((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

	const char* InputCString = InputGLSL.c_str();
	const char* fname = filename.c_str();
	EShLanguage ShaderType = FindLanguage(filename);
	glslang::TShader Shader(ShaderType);
	// Shader.setStrings(&InputCString, 1);
	Shader.setStringsWithLengthsAndNames(&InputCString, nullptr, &fname, 1);


	const int ShaderLanguageVersion = 450;
	const EShTargetClientVersion VulkanVersion = glslang::EShTargetVulkan_1_1;
	const EShTargetLanguageVersion SPIRVVersion = glslang::EShTargetSpv_1_3;

	Shader.setEnvInput(EShSourceGlsl, ShaderType, EShClientVulkan, VulkanVersion);
	Shader.setEnvClient(EShClientVulkan, VulkanVersion);
	Shader.setEnvTarget(EShTargetSpv, SPIRVVersion);

	DirStackFileIncluder Includer;

	// TODO: ShaderPod
	// use std::filesystem
	Includer.pushExternalLocalDirectory("");
	Includer.pushExternalLocalDirectory("./engine-data/spv/");


	std::string PreprocessedGLSL;

	if (!Shader.preprocess(&DefaultTBuiltInResource, ShaderLanguageVersion, ENoProfile, false, false,
			(EShMessages)(EShMsgSpvRules | EShMsgVulkanRules), &PreprocessedGLSL, Includer)) {
		auto er = fmt::format(
			"\nGLSL preprocessing failed: {}.\n{}\n{}\n", filename, Shader.getInfoLog(), Shader.getInfoDebugLog());
		// AppendErrors += er;
		LOG_ERROR("{}", er);
		return {};
	}

	const char* GLSLcharArray = PreprocessedGLSL.c_str();
	Shader.setStrings(&GLSLcharArray, 1);

	if (!Shader.parse(
			&DefaultTBuiltInResource, ShaderLanguageVersion, true, (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules))) {
		auto er = fmt::format(
			"\nGLSL parsing failed: {}.\n{}\n{}\n", filename, Shader.getInfoLog(), Shader.getInfoDebugLog());
		// AppendErrors += er;
		LOG_ERROR("{}", er);
		return {};
	}

	TProgram Program;
	Program.addShader(&Shader);

	if (!Program.link((EShMessages)(EShMsgSpvRules | EShMsgVulkanRules))) {
		auto er = fmt::format(
			"\nGLSL linking failed: {}.\n{}\n{}\n", filename, Shader.getInfoLog(), Shader.getInfoDebugLog());
		// AppendErrors += er;
		LOG_ERROR("{}", er);
		return {};
	}

	spv::SpvBuildLogger spvLogger;
	glslang::SpvOptions spvOptions;
	spvOptions.disableOptimizer = true;
	spvOptions.generateDebugInfo = true;
	spvOptions.validate = true;

	glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), outCode, &spvLogger, &spvOptions);

	for (auto msg : spvLogger.getAllMessages()) {
		LOG_REPORT("Shader Compiling: {}", msg);
	}

	// AppendErrors += fmt::format("{}...\n", filename);

	// TODO: FIX glslang::FinalizeProcess
	return outCode;
}


EShLanguage FindLanguage(const std::string& filename)
{
	std::string stageName;
	// Note: "first" extension means "first from the end", i.e.
	// if the file is named foo.vert.glsl, then "glsl" is first,
	// "vert" is second.
	size_t firstExtStart = filename.find_last_of(".");
	bool hasFirstExt = firstExtStart != std::string::npos;

	size_t secondExtStart = hasFirstExt ? filename.find_last_of(".", firstExtStart - 1) : std::string::npos;

	bool hasSecondExt = secondExtStart != std::string::npos;

	std::string firstExt = filename.substr(firstExtStart + 1, std::string::npos);
	bool usesUnifiedExt = hasFirstExt && (firstExt == "glsl" || firstExt == "hlsl");

	if (usesUnifiedExt && firstExt == "hlsl") {
		LOG_WARN("Found hlsl shader: {} .", filename);
	}
	if (hasFirstExt && !usesUnifiedExt) {
		stageName = firstExt;
	}
	else if (usesUnifiedExt && hasSecondExt) {
		stageName = filename.substr(secondExtStart + 1, firstExtStart - secondExtStart - 1);
	}
	else {
		LOG_ERROR("Failed to determine shader stage: {}", filename);
		return EShLangVertex;
	}


	if (stageName == "vert")
		return EShLangVertex;
	else if (stageName == "tesc")
		return EShLangTessControl;
	else if (stageName == "tese")
		return EShLangTessEvaluation;
	else if (stageName == "geom")
		return EShLangGeometry;
	else if (stageName == "frag")
		return EShLangFragment;
	else if (stageName == "comp")
		return EShLangCompute;
	else if (stageName == "rgen")
		return EShLangRayGenNV;
	else if (stageName == "rint")
		return EShLangIntersectNV;
	else if (stageName == "rahit")
		return EShLangAnyHitNV;
	else if (stageName == "rchit")
		return EShLangClosestHitNV;
	else if (stageName == "rmiss")
		return EShLangMissNV;
	else if (stageName == "rcall")
		return EShLangCallableNV;
	else if (stageName == "mesh")
		return EShLangMeshNV;
	else if (stageName == "task")
		return EShLangTaskNV;

	LOG_ERROR("Failed to determine shader stage: {}", filename);
	return EShLangVertex;
}
