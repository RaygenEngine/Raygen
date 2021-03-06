#pragma once
#include "assets/importers/PodImporter.h"

struct CubemapImporter : public PodImporter<Cubemap> {
	CubemapImporter(std::string_view name)
		: PodImporter({ ".cmp" }, name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
};
