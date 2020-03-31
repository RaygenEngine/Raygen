#pragma once
#include "assets/importers/PodImporter.h"
#include "assets/pods/ShaderPod.h"
#include "assets/util/SpirvCompiler.h"

struct ShaderImporter : public PodImporter<Shader> {
	ShaderImporter(std::string_view name)
		: PodImporter<Shader>(
			{
				".vert",
				".tesc",
				".tese",
				".geom",
				".frag",
				".comp",
				".rgen",
				".rint",
				".rahit",
				".rchit",
				".rmiss",
				".rcall",
				".mesh",
				".task",
			},
			name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
};

namespace shd {
// WIP:
ShaderStage LoadAndCompileStage(
	const std::string& pathNoExt, const std::string& ext, TextCompilerErrors* outErrors = nullptr);
} // namespace shd
