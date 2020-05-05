#include "pch.h"
#include "SpirvCompiler.h"

#include "engine/Logger.h"
#include "engine/Timer.h"
#include "core/StringConversions.h"
#include "assets/util/SpirvReflector.h"

#include <glslang/OSDependent/osinclude.h>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <StandAlone/ResourceLimits.h>

#include <thread>
#include <mutex>

EShLanguage FindLanguage(const std::string& filename);
EShLanguage LangFromStage(ShaderStageType type);


std::vector<uint32> CompileImpl(
	const std::string& code, const std::string& shadername, TextCompilerErrors* outError, EShLanguage stage)
{
	using namespace glslang;

	auto reportError = [&](TShader& shader) {
		if (!outError) {
			auto er = fmt::format("\nGLSL Compiler Error: {}.\n===\n{}\n====", shadername, shader.getInfoLog());
			LOG_ERROR("{}", er);
		}
		else {
			// auto er = fmt::format("\nGLSL Compiler Error: {}.\n===\n{}\n====", shadername, shader.getInfoLog());

			// PERF: can be done with just string_views

			std::stringstream ss;
			ss.str(shader.getInfoLog());
			std::string item;

			// Parsing example:
			// ERROR: gbuffer.frag:39: '' :  syntax error, unexpected IDENTIFIER
			// We want to parse the line number

			while (std::getline(ss, item)) {
				using namespace std::literals;
				constexpr std::string_view errorStr = "ERROR: "sv;
				constexpr size_t size = errorStr.size();

				if (item.starts_with(errorStr)) {
					std::string_view view{ item.c_str() };
					view = view.substr(size);

					auto loc = view.find_first_of(':');
					if (loc == std::string::npos) {
						continue;
					}

					view = view.substr(loc + 1);

					// Find Next ':'
					loc = view.find_first_of(':');
					if (loc == std::string::npos) {
						continue;
					}

					auto numView = view.substr(0, loc);
					int32 errorLine = str::fromStrView<int32>(numView);

					view = view.substr(loc + 1);
					outError->errors.emplace(errorLine, std::string(view));
				}
			}
		}
	};


	std::vector<uint32> outCode;

	static bool HasInitGlslLang = false;
	if (!HasInitGlslLang) {
		static std::mutex m;
		std::lock_guard lock(m);
		if (!HasInitGlslLang) {
			HasInitGlslLang = true;
			glslang::InitializeProcess();
		}
	}

	const char* InputCString = code.c_str();
	const char* fname = shadername.c_str();
	EShLanguage ShaderType = stage;
	glslang::TShader Shader(ShaderType);
	// Shader.setStrings(&InputCString, 1);
	Shader.setStringsWithLengthsAndNames(&InputCString, nullptr, &fname, 1);


	const int ShaderLanguageVersion = 450;
	const EShTargetClientVersion VulkanVersion = glslang::EShTargetVulkan_1_1; // CHECK: Update for KHR RT
	const EShTargetLanguageVersion SPIRVVersion = glslang::EShTargetSpv_1_3;

	Shader.setEnvInput(EShSourceGlsl, ShaderType, EShClientVulkan, VulkanVersion);
	Shader.setEnvClient(EShClientVulkan, VulkanVersion);
	Shader.setEnvTarget(EShTargetSpv, SPIRVVersion);

	DirStackFileIncluder Includer;

	// CHECK: Shader
	// use std::filesystem
	Includer.pushExternalLocalDirectory("");
	Includer.pushExternalLocalDirectory("./engine-data/spv/");


	std::string PreprocessedGLSL;

	if (!Shader.preprocess(&DefaultTBuiltInResource, ShaderLanguageVersion, ENoProfile, false, false,
			(EShMessages)(EShMsgSpvRules | EShMsgVulkanRules), &PreprocessedGLSL, Includer)) {
		reportError(Shader);
		return {};
	}

	const char* GLSLcharArray = PreprocessedGLSL.c_str();
	Shader.setStrings(&GLSLcharArray, 1);
	if (!Shader.parse(
			&DefaultTBuiltInResource, ShaderLanguageVersion, true, (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules))) {
		reportError(Shader);
		return {};
	}

	TProgram Program;
	Program.addShader(&Shader);

	if (!Program.link((EShMessages)(EShMsgSpvRules | EShMsgVulkanRules))) {
		auto er = fmt::format(
			"\nGLSL linking failed: {}.\n{}\n{}\n", shadername, Shader.getInfoLog(), Shader.getInfoDebugLog());
		LOG_ERROR("{}", er);
		return {};
	}


	spv::SpvBuildLogger spvLogger;
	glslang::SpvOptions spvOptions;
	spvOptions.disableOptimizer = true;
	spvOptions.generateDebugInfo = false;
	spvOptions.validate = true;

	glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), outCode, &spvLogger, &spvOptions);

	for (auto msg : spvLogger.getAllMessages()) {
		LOG_REPORT("Shader Compiling: {}", msg);
	}

	// AppendErrors += fmt::format("{}...\n", filename);

	// CHECK: FIX glslang::FinalizeProcess

	return outCode;
}


std::vector<uint32> ShaderCompiler::Compile(
	const std::string& code, const std::string& shadername, TextCompilerErrors* outError)
{
	return CompileImpl(code, shadername, outError, FindLanguage(shadername));
}

std::vector<uint32> ShaderCompiler::Compile(
	const std::string& code, ShaderStageType type, const std::string& shadername, TextCompilerErrors* outError)
{
	return CompileImpl(code, shadername, outError, LangFromStage(type));
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

EShLanguage LangFromStage(ShaderStageType type)
{
	// TODO: Use non NV versions of the enum
	switch (type) {
		case ShaderStageType::Vertex: return EShLangVertex;
		case ShaderStageType::TessControl: return EShLangTessControl;
		case ShaderStageType::TessEvaluation: return EShLangTessEvaluation;
		case ShaderStageType::Geometry: return EShLangGeometry;
		case ShaderStageType::Fragment: return EShLangFragment;
		case ShaderStageType::Compute: return EShLangCompute;
		case ShaderStageType::RayGen: return EShLangRayGenNV;
		case ShaderStageType::Intersect: return EShLangIntersectNV;
		case ShaderStageType::AnyHit: return EShLangAnyHitNV;
		case ShaderStageType::ClosestHit: return EShLangClosestHitNV;
		case ShaderStageType::Miss: return EShLangMissNV;
		case ShaderStageType::Callable: return EShLangCallableNV;
	}
	return EShLangVertex;
}
