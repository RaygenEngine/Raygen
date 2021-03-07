#include "ImageImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/pods/Image.h"
#include "assets/util/ImageUtl.h"

BasePodHandle ImageImporter::Import(const fs::path& path)
{
	const auto finalPath = path.generic_string();

	auto&& [handle, pod]
		= AssetImporterManager->CreateEntry<Image>(path.generic_string(), path.filename().replace_extension().string());

	stbaux::LoadImage(finalPath.c_str(), pod->format, pod->width, pod->height, pod->data);


	return handle;
}
