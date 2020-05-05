#pragma once
#include "assets/importers/PodImporter.h"
#include "assets/pods/Shader.h"
#include "assets/util/SpirvCompiler.h"

struct ShaderStageImporter : public PodImporter<ShaderStage> {
	ShaderStageImporter(std::string_view name)
		: PodImporter<ShaderStage>(
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
			},
			name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
	void Reimport(PodEntry* intoEntry, const uri::Uri& uri) override;

private:
	void CompilePod(ShaderStage* pod, const uri::Uri& uri);
};


struct ShaderImporter : public PodImporter<Shader> {
	ShaderImporter(std::string_view name)
		: PodImporter<Shader>(
			{
				".shader",
			},
			name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
	// void Reimport(PodEntry* intoEntry, const uri::Uri& uri) override;
};
