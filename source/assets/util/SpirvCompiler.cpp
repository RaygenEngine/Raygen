#include "SpirvCompiler.h"

#include "engine/console/ConsoleVariable.h"
#include "platform/DynLibLoader.h"

#include <StandAlone/ResourceLimits.h>
#include <StandAlone/SharedLibCombined/SharedLibCombined.h>
#include <iostream>
#include <mutex>

EShLanguage FindLanguage(const std::string& filename);
EShLanguage LangFromStage(ShaderStageType type);


namespace {
struct SpirvSharedLib : DynLibLoader {
	SpirvSharedLib()
		: DynLibLoader("../Release/SPIRV-SharedLib", true)
	{
		if (HasLoadedLibrary()) {
			std::cout << "===== RAYGEN: USING DLL SHADER COMPILER ====\n";
			LIBLOAD_INTO(spirvc_init_process);
			LIBLOAD_INTO(spirvc_finalize_process);
			LIBLOAD_INTO(spirvc_compile);
			LIBLOAD_INTO(spirvc_free_compilation);
			pfn_spirvc_init_process();
		}
	}

	~SpirvSharedLib()
	{
		if (pfn_spirvc_finalize_process) {
			pfn_spirvc_finalize_process();
		}
	}

	PFN_FUNC(spirvc_init_process);
	PFN_FUNC(spirvc_finalize_process);
	PFN_FUNC(spirvc_compile);
	PFN_FUNC(spirvc_free_compilation);
};
// TODO: move this outside of global scope
static SpirvSharedLib lib;
} // namespace


void ReportError(const char* infoLog, const std::string& shadername, TextCompilerErrors* outError);

std::vector<uint32> CompileImplDLL(
	const std::string& code, const std::string& shadername, TextCompilerErrors* outError, EShLanguage stage)
{
	using namespace glslang;
	constexpr std::array includes = { "", "./engine-data/spv/", "./engine-data/spv/includes" };

	ShaderCompileInfo info;
	info.shaderType = stage;
	info.inputCode = code.c_str();
	info.shadername = shadername.c_str();
	// The rest are defaults

	info.shaderLanguageVersion = 460;
	info.language = glslang::EShSourceGlsl;
	info.client = glslang::EShClientVulkan;
	info.targetClientVersion = glslang::EShTargetVulkan_1_2;
	info.targetLanguage = glslang::EShTargetSpv;
	info.targetLanguageVersion = glslang::EShTargetSpv_1_5;

	info.localIncludeDirs = includes.data();
	info.localIncludeDirCount = includes.size();

	info.errorMessages = (EShMessages)(EShMsgSpvRules | EShMsgVulkanRules);

	auto& options = info.spvOptions;
	options.generateDebugInfo = false;
	options.disableOptimizer = true;
	options.validate = false;
	options.optimizeSize = false;

	// Handle spirvc_compile(ShaderCompileInfo * compileInfo);
	// void spirvc_free_compilation(Handle handle);

	Handle result = lib.pfn_spirvc_compile(&info);

	if (!result->parseSuccess) {
		ReportError(result->infoLog, shadername, outError);
		lib.pfn_spirvc_free_compilation(result);
		return {};
	}
	if (!result->linkSuccess) {
		auto er
			= fmt::format("\nGLSL linking failed: {}.\n{}\n{}\n", shadername, result->infoLog, result->infoDebugLog);
		LOG_ERROR("{}", er);
		lib.pfn_spirvc_free_compilation(result);
		return {};
	}

	if (result->allMessages) {
		LOG_REPORT("Shader Compiling Messages: {}", result->allMessages);
	}


	// Copy code back
	std::vector<uint32> outCode(result->spvResultSize);


	if (result->spvResultSize != 0) {
		memcpy(outCode.data(), result->spvResult, outCode.size() * sizeof(uint32));
	}

	if (outError && outCode.size() > 0) {
		outError->wasSuccessful = true;
	}

	lib.pfn_spirvc_free_compilation(result);
	return outCode;
}

std::vector<uint32> CompileImplStaticLink(
	const std::string& code, const std::string& shadername, TextCompilerErrors* outError, EShLanguage stage)
{
	using namespace glslang;


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
		ReportError(Shader.getInfoLog(), shadername, outError);
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


	auto messages = spvLogger.getAllMessages();
	if (!messages.empty()) {
		LOG_REPORT("Shader Compiling: {}", messages);
	}

	// AppendErrors += fmt::format("{}...\n", filename);

	// CHECK: FIX glslang::FinalizeProcess

	if (outError && outCode.size() > 0) {
		outError->wasSuccessful = true;
	}

	return outCode;
}


std::vector<uint32> CompileImplSelect(
	const std::string& code, const std::string& shadername, TextCompilerErrors* outError, EShLanguage stage)
{

	if (lib.HasLoadedLibrary()) {
		return CompileImplDLL(code, shadername, outError, stage);
	}
	return CompileImplStaticLink(code, shadername, outError, stage);
}

std::vector<uint32> ShaderCompiler::Compile(
	const std::string& code, const std::string& shadername, TextCompilerErrors* outError)
{
	return CompileImplSelect(code, shadername, outError, FindLanguage(shadername));
}

std::vector<uint32> ShaderCompiler::Compile(
	const std::string& code, ShaderStageType type, const std::string& shadername, TextCompilerErrors* outError)
{
	return CompileImplSelect(code, shadername, outError, LangFromStage(type));
}

std::vector<uint32> ShaderCompiler::Compile(const std::string& code, ShaderStageType type, TextCompilerErrors* outError)
{
	return CompileImplSelect(code, "custom", outError, LangFromStage(type));
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

ConsoleVariable<bool> cons_compilerReportAll{ "a.shaders.compiler.reportAllErrors", false,
	"Enable error reporting in console for ALL shaders compiled at all times." };

void ReportError(const char* infoLog, const std::string& shadername, TextCompilerErrors* outError)
{
	if (!outError || cons_compilerReportAll) {
		auto er = fmt::format("\nGLSL Compiler Error: {}.\n===\n{}\n====", shadername, infoLog);
		LOG_ERROR("{}", er);

		if (!outError) {
			return;
		}
	}
	// auto er = fmt::format("\nGLSL Compiler Error: {}.\n===\n{}\n====", shadername, shader.getInfoLog());

	// PERF: can be done with just string_views

	std::stringstream ss;
	ss.str(infoLog);
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
