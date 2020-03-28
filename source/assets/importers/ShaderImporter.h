#pragma once
#include "assets/importers/PodImporter.h"

struct ShaderImporter : public PodImporter<Image> {
	ShaderImporter(std::string_view name)
		: PodImporter<Image>(
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
