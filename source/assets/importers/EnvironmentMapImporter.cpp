#include "EnvironmentMapImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/pods/EnvironmentMap.h"

BasePodHandle EnvironmentMapImporter::Import(const fs::path& path)
{
	const auto finalPath = path.generic_string();
	auto p = path;

	auto& [handle, pod] = AssetImporterManager->CreateEntry<EnvironmentMap>(
		path.generic_string(), path.filename().replace_extension().string());

	AssetImporterManager->PushPath(path.filename().replace_extension());
	auto skybox = AssetImporterManager->ImportRequest<Cubemap>(p.replace_extension(".cmp"));
	AssetImporterManager->PopPath();

	pod->skybox = skybox;

	return handle;
}
