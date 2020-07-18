#pragma once
#include "assets/importers/PodImporter.h"

struct ImageImporter : public PodImporter<Image> {
	ImageImporter(std::string_view name)
		: PodImporter({ ".png", ".jpg", ".jpeg", ".tiff", ".tga", ".hdr", ".bmp" }, name)
	{
	}

	[[nodiscard]] BasePodHandle Import(const fs::path& path) override;
};
