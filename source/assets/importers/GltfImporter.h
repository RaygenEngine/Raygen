#pragma once
#include "assets/importers/PodImporter.h"

struct GltfImporter : public PodImporter<Mesh> {
	GltfImporter(std::string_view name)
		: PodImporter({ ".gltf" }, name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
};
