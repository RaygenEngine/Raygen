#pragma once
#include "assets/importers/PodImporter.h"
#include "assets/pods/ModelPod.h"

struct GltfImporter : public PodImporter<Model> {
	GltfImporter(std::string_view name)
		: PodImporter({ ".gltf" }, name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
};
