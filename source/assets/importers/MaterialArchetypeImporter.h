#pragma once
#include "assets/importers/PodImporter.h"


struct MaterialArchetypeImporter : public PodImporter<MaterialArchetype> {
	MaterialArchetypeImporter(std::string_view name)
		: PodImporter<MaterialArchetype>({ ".glsl", ".rmat" }, name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
};
