#include "GlslCompiler.h"

#include "core/StringConversions.h"

#include <SPIRV/GlslangToSpv.h>
#include <StandAlone/DirStackFileIncluder.h>
#include <StandAlone/ResourceLimits.h>
#include <mutex>

namespace {
EShLanguage LangFromStage(ShaderStageType type)
{
	switch (type) {
		case ShaderStageType::Vertex: return EShLangVertex;
		case ShaderStageType::TessControl: return EShLangTessControl;
		case ShaderStageType::TessEvaluation: return EShLangTessEvaluation;
		case ShaderStageType::Geometry: return EShLangGeometry;
		case ShaderStageType::Fragment: return EShLangFragment;
		case ShaderStageType::Compute: return EShLangCompute;
		case ShaderStageType::RayGen: return EShLangRayGen;
		case ShaderStageType::Intersect: return EShLangIntersect;
		case ShaderStageType::AnyHit: return EShLangAnyHit;
		case ShaderStageType::ClosestHit: return EShLangClosestHit;
		case ShaderStageType::Miss: return EShLangMiss;
		case ShaderStageType::Callable: return EShLangCallable;
	}
	return EShLangVertex;
}
} // namespace


void CompileGlslImpl(LogTransactions* log, std::vector<uint32>* outCode, const std::string* inCode,
	const std::string* shaderNamePtr, TextCompilerErrors* outError, ShaderStageType stage)
{
	const std::string& code = *inCode;

	using namespace glslang;

	auto reportError = [&](TShader& shader) {
		if (!outError) {
			auto er = fmt::format("\nGLSL Compiler Error: {}.\n===\n{}\n====", *shaderNamePtr, shader.getInfoLog());
			log->Error(std::move(er));
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
	const char* fname = shaderNamePtr->c_str();
	EShLanguage ShaderType = LangFromStage(stage);
	glslang::TShader Shader(ShaderType);
	Shader.setStringsWithLengthsAndNames(&InputCString, nullptr, &fname, 1);


	const int ShaderLanguageVersion = 460;
	const EShTargetClientVersion VulkanVersion = glslang::EShTargetVulkan_1_2;
	const EShTargetLanguageVersion SPIRVVersion = glslang::EShTargetSpv_1_5;

	Shader.setEnvInput(EShSourceGlsl, ShaderType, EShClientVulkan, VulkanVersion);
	Shader.setEnvClient(EShClientVulkan, VulkanVersion);
	Shader.setEnvTarget(EShTargetSpv, SPIRVVersion);


	DirStackFileIncluder Includer;

	// CHECK: Shader
	// use std::filesystem
	Includer.pushExternalLocalDirectory("");
	Includer.pushExternalLocalDirectory("./engine-data/spv/");
	Includer.pushExternalLocalDirectory("./engine-data/spv/includes");


	if (!Shader.parse(&DefaultTBuiltInResource, ShaderLanguageVersion, true,
			(EShMessages)(EShMsgSpvRules | EShMsgVulkanRules), Includer)) {
		reportError(Shader);
		outCode->clear();
		return;
	}

	TProgram Program;
	Program.addShader(&Shader);

	if (!Program.link((EShMessages)(EShMsgSpvRules | EShMsgVulkanRules))) {
		log->Error(fmt::format(
			"\nGLSL linking failed: {}.\n{}\n{}\n", *shaderNamePtr, Shader.getInfoLog(), Shader.getInfoDebugLog()));
		outCode->clear();
		return;
	}

	spv::SpvBuildLogger spvLogger;
	glslang::SpvOptions spvOptions;
	spvOptions.disableOptimizer = true;
	spvOptions.generateDebugInfo = false;
	spvOptions.validate = false;
	spvOptions.optimizeSize = false;

	glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), *outCode, &spvLogger, &spvOptions);


	for (auto msg : spvLogger.getAllMessages()) {
		log->Warn(fmt::format("Shader Compiling: {}", msg));
	}

	// AppendErrors += fmt::format("{}...\n", filename);

	// CHECK: FIX glslang::FinalizeProcess

	if (outError && outCode->size() > 0) {
		outError->wasSuccessful = true;
	}
}
