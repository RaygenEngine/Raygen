#include "SpirvCompiler.h"

#include "core/StringConversions.h"
#include "engine/Timer.h"

#include "dynlib/GlslCompiler.h"

#include <fstream>


ShaderStageType FindLanguage(const std::string& filename);

std::vector<uint32> ShaderCompiler::Compile(
	const std::string& code, const std::string& shadername, TextCompilerErrors* outError)
{
	std::vector<uint32> result;
	LogTransactions log;
	CompileGlslImpl(&log, &result, &code, &shadername, outError, FindLanguage(shadername));
	return result;
}

std::vector<uint32> ShaderCompiler::Compile(
	const std::string& code, ShaderStageType type, const std::string& shadername, TextCompilerErrors* outError)
{
	std::vector<uint32> result;
	LogTransactions log;
	CompileGlslImpl(&log, &result, &code, &shadername, outError, type);
	return result;
}

std::vector<uint32> ShaderCompiler::Compile(const std::string& code, ShaderStageType type, TextCompilerErrors* outError)
{
	std::vector<uint32> result;
	std::string name = "custom";
	LogTransactions log;
	CompileGlslImpl(&log, &result, &code, &name, outError, type);
	return result;
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
	auto str = StringFromFile(filepath);
	return Compile(str, filepath, outError);
}


ShaderStageType FindLanguage(const std::string& filename)
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
		return ShaderStageType::Vertex;
	}


	if (stageName == "vert")
		return ShaderStageType::Vertex;
	else if (stageName == "tesc")
		return ShaderStageType::TessControl;
	else if (stageName == "tese")
		return ShaderStageType::TessEvaluation;
	else if (stageName == "geom")
		return ShaderStageType::Geometry;
	else if (stageName == "frag")
		return ShaderStageType::Fragment;
	else if (stageName == "comp")
		return ShaderStageType::Compute;
	else if (stageName == "rgen")
		return ShaderStageType::RayGen;
	else if (stageName == "rint")
		return ShaderStageType::Intersect;
	else if (stageName == "rahit")
		return ShaderStageType::AnyHit;
	else if (stageName == "rchit")
		return ShaderStageType::ClosestHit;
	else if (stageName == "rmiss")
		return ShaderStageType::Miss;
	else if (stageName == "rcall")
		return ShaderStageType::Callable;
	// else if (stageName == "mesh")
	//	return EShLangMeshNV;
	// else if (stageName == "task")
	//	return EShLangTaskNV;

	LOG_ERROR("Failed to determine shader stage: {}", filename);
	return ShaderStageType::Vertex;
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
