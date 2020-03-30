#pragma once
#include "assets/importers/PodImporter.h"

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
