#pragma once
#include "asset/importers/PodImporter.h"
#include "asset/pods/ImagePod.h"

struct ImageImporter : public PodImporter<ImagePod> {
	ImageImporter(std::string_view name)
		: PodImporter({ ".png", ".jpg", ".jpeg", ".tiff", ".tga", ".hdr", ".bmp" }, name)
	{
	}

	BasePodHandle Import(const fs::path& path) override;
};
