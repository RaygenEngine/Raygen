#include "pch.h"
#include "EnvironmentMapImporter.h"

#include "assets/AssetImporterManager.h"
#include "assets/util/ImageUtl.h"

#include <fstream>

BasePodHandle EnvironmentMapImporter::Import(const fs::path& path)
{
	const auto finalPath = path.generic_string();
	auto p = path;

	auto& [handle, pod] = ImporterManager->CreateEntry<EnvironmentMap>(
		path.generic_string(), path.filename().replace_extension().string());

	ImporterManager->PushPath(path.filename().replace_extension());
	auto skybox = ImporterManager->ImportRequest<Cubemap>(p.replace_extension(".cmp"));
	ImporterManager->PopPath();

	pod->skybox = skybox;

	return handle;
}
