#pragma once
#include "assets/shared/ShaderStageShared.h"
#include "assets/util/DynamicDescriptorSet.h"

struct ShaderCompiler {
	static std::vector<uint32> Compile(
		const std::string& code, const std::string& shadername, TextCompilerErrors* outError = nullptr);

	static std::vector<uint32> Compile(const std::string& code, ShaderStageType type, const std::string& shadername,
		TextCompilerErrors* outError = nullptr);

	static std::vector<uint32> Compile(
		const std::string& code, ShaderStageType type, TextCompilerErrors* outError = nullptr);

	static std::vector<uint32> Compile(const std::string& filepath, TextCompilerErrors* outError = nullptr);


	static DynamicDescriptorSetLayout GenerateUboClass(
		const std::vector<std::string>& lines, TextCompilerErrors* outErrors = nullptr);

	static DynamicDescriptorSetLayout GenerateUboClass(
		const std::vector<std::string_view>& lines, TextCompilerErrors* outErrors = nullptr);
};

namespace shd {

inline std::string StageToExt(ShaderStageType type)
{
	switch (type) {
		case ShaderStageType::Vertex: return ".vert";
		case ShaderStageType::Fragment: return ".frag";
		case ShaderStageType::RayGen: return ".rgen";
		case ShaderStageType::Intersect: return ".rint";
		case ShaderStageType::AnyHit: return ".rahit";
		case ShaderStageType::ClosestHit: return ".rchit";
		case ShaderStageType::Miss: return ".rmiss";
		case ShaderStageType::Geometry: return ".geom";
		case ShaderStageType::TessControl: return ".tesc";
		case ShaderStageType::TessEvaluation: return ".tese";
		case ShaderStageType::Callable: return ".rcall";
		case ShaderStageType::Compute: return ".comp";
	}
	return "";
}

inline ShaderStageType ExtToStage(std::string_view extName)
{
	extName = extName.substr(1);

	if (extName == "vert")
		return ShaderStageType::Vertex;
	else if (extName == "tesc")
		return ShaderStageType::TessControl;
	else if (extName == "tese")
		return ShaderStageType::TessEvaluation;
	else if (extName == "geom")
		return ShaderStageType::Geometry;
	else if (extName == "frag")
		return ShaderStageType::Fragment;
	else if (extName == "comp")
		return ShaderStageType::Compute;
	else if (extName == "rgen")
		return ShaderStageType::RayGen;
	else if (extName == "rint")
		return ShaderStageType::Intersect;
	else if (extName == "rahit")
		return ShaderStageType::AnyHit;
	else if (extName == "rchit")
		return ShaderStageType::ClosestHit;
	else if (extName == "rmiss")
		return ShaderStageType::Miss;
	else if (extName == "rcall")
		return ShaderStageType::Callable;
	return ShaderStageType::Vertex;
}


} // namespace shd
