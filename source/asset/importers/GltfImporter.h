#pragma once
#include "asset/importers/PodImporter.h"
#include "asset/pods/ModelPod.h"

struct GltfImporter : public PodImporter<ModelPod> {
	GltfImporter(std::string_view name)
		: PodImporter({ ".gltf" }, name)
	{
	}

	BasePodHandle Import(const fs::path& path) override;
};
