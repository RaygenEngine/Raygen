#include "SpirvCompiler.h"

#include "engine/Logger.h"
#include "engine/Timer.h"
#include "core/StringConversions.h"
#include "assets/util/SpirvReflector.h"
#include "assets/AssetRegistry.h"
#include "engine/console/ConsoleVariable.h"

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
	spvOptions.validate = false;
	spvOptions.optimizeSize = false;

	glslang::GlslangToSpv(*Program.getIntermediate(ShaderType), outCode, &spvLogger, &spvOptions);

	for (auto msg : spvLogger.getAllMessages()) {
		LOG_REPORT("Shader Compiling: {}", msg);
	}

	// AppendErrors += fmt::format("{}...\n", filename);

	// CHECK: FIX glslang::FinalizeProcess

	if (outError && outCode.size() > 0) {
		outError->wasSuccessful = true;
	}

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

std::vector<uint32> ShaderCompiler::Compile(const std::string& code, ShaderStageType type, TextCompilerErrors* outError)
{
	return CompileImpl(code, "custom", outError, LangFromStage(type));
}

std::string StringFromFile(const std::string& path)
{
	std::ifstream t(path);
	t.seekg(0, std::ios::end);
	size_t size = t.tellg();
	std::string buffer(size, ' ');
	t.seekg(0);
	t.read(&buffer[0], size);
	return buffer;
}

std::vector<uint32> ShaderCompiler::Compile(const std::string& filepath, TextCompilerErrors* outError)
{
	return Compile(StringFromFile(filepath), filepath, outError);
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


namespace {

template<typename StringLike>
DynamicDescriptorSetLayout GenerateUboClassStr(const std::vector<StringLike>& lines, TextCompilerErrors* outErrors)
{
	DynamicDescriptorSetLayout layout;

	std::map<int, std::string> errors;

	std::unordered_set<std::string, str::Hash> identifiers;
	layout = {};

	for (uint32 lineNum = 0; auto& line : lines) {
		lineNum++;
		if (line.starts_with("//") || line.size() < 1) {
			continue;
		}

		auto parts = str::split(line, " ;");
		if (parts.size() < 2) {
			errors.emplace(lineNum, "Expected format for each line is: 'type name;'");
			continue;
		}


		if (*(parts[1].data() + parts[1].size()) != ';') {
			errors.emplace(lineNum, "Expected a ';'");
			continue;
		}

		// PERF: Do hashing on strview
		std::string id = std::string(parts[1]);
		if (identifiers.contains(id)) {
			errors.emplace(lineNum, fmt::format("Duplicate identifier: {}.", id));
			continue;
		}

		identifiers.insert(id);

		if (str::equal(parts[0], "vec4")) {
			layout.uboClass.AddProperty<glm::vec4>(id);
		}
		else if (str::equal(parts[0], "col4")) {
			layout.uboClass.AddProperty<glm::vec4>(id, PropertyFlags::Color);
		}
		else if (str::equal(parts[0], "int")) {
			layout.uboClass.AddProperty<int>(id);
		}
		else if (str::equal(parts[0], "float")) {
			layout.uboClass.AddProperty<float>(id);
		}
		else if (str::equal(parts[0], "ubo")) {
			layout.uboName = id;
		}
		else if (str::equal(parts[0], "sampler2d")) {
			layout.samplers2d.push_back(id);
		}
		else {
			errors.emplace(lineNum, "Unknown variable type.");
			continue;
		}
	}

	if (outErrors) {
		outErrors->errors = std::move(errors);
		outErrors->wasSuccessful = outErrors->errors.size() == 0;
	}
	else {
		if (outErrors->errors.size() > 0) {
			LOG_ERROR("Errors while compiling UBO code: ");
			for (auto& [line, error] : errors) {
				LOG_ERROR("Line {}: {} ({})", line, error, lines[line]);
			}
		}
	}

	return layout;
}
} // namespace

DynamicDescriptorSetLayout ShaderCompiler::GenerateUboClass(
	const std::vector<std::string>& lines, TextCompilerErrors* outErrors)
{
	return GenerateUboClassStr(lines, outErrors);
}

DynamicDescriptorSetLayout ShaderCompiler::GenerateUboClass(
	const std::vector<std::string_view>& lines, TextCompilerErrors* outErrors)
{
	return GenerateUboClassStr(lines, outErrors);
}
