#pragma once
#include "assets/importers/PodImporter.h"


struct MaterialInstanceImporter : public PodImporter<MaterialInstance> {
	MaterialInstanceImporter(std::string_view name)
		: PodImporter<MaterialInstance>({ ".matinst" }, name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
};
