#include "ImageImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/pods/Image.h"
#include "assets/util/ImageUtl.h"

BasePodHandle ImageImporter::Import(const fs::path& path)
{
	const auto finalPath = path.generic_string();

	auto [handle, pod]
		= AssetImporterManager->CreateEntry<Image>(path.generic_string(), path.filename().replace_extension().string());

	stbaux::LoadImage(finalPath.c_str(), pod->format, pod->width, pod->height, pod->data);

	// CHECK: fix sponza floor normal map <-- remove
	if (path.filename() == "14267839433702832875.jpg") {

		for (int32 i = 0; i < pod->data.size(); i += 4) {
			pod->data[i] = byte(255) - pod->data[i];
		}
	}

	return handle;
}
