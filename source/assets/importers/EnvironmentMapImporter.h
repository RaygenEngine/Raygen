#pragma once
#include "assets/importers/PodImporter.h"
#include "assets/pods/EnvironmentMap.h"

struct EnvironmentMapImporter : public PodImporter<EnvironmentMap> {
	EnvironmentMapImporter(std::string_view name)
		: PodImporter({ ".env" }, name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
};
