#pragma once
#include "assets/importers/PodImporter.h"
#include "assets/pods/ModelPod.h"

struct GltfImporter : public PodImporter<ModelPod> {
	GltfImporter(std::string_view name)
		: PodImporter({ ".gltf" }, name)
	{
	}

	BasePodHandle Import(const fs::path& path) override;
};
