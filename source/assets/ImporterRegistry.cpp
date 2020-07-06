#include "pch.h"
#include "ImporterRegistry.h"

#include "assets/importers/GltfImporter.h"
#include "assets/importers/ImageImporter.h"
#include "assets/importers/ShaderImporter.h"
#include "assets/importers/CubemapImporter.h"
#include "assets/importers/EnvironmentMapImporter.h"
#include "assets/AssetImporterManager.h"
#include <fstream>

ImporterRegistry::ImporterRegistry()
{
	RegisterImporters<ImageImporter, GltfImporter, ShaderStageImporter, ShaderImporter, CubemapImporter,
		EnvironmentMapImporter>();
}

void ImporterRegistry::ReimportEntry(PodEntry* entry)
{
	uri::Uri importUri = entry->metadata.originalImportLocation;
	if (importUri.empty()) {
		LOG_WARN("Reimporting entry without import location: {}", entry->path);
		return;
	}
	std::string fileExt = std::string(uri::GetDiskExtension(importUri));

	if (!fs::exists(uri::GetDiskPath(importUri))) {
		LOG_WARN(
			"Failed to find file: {} during reimport. Aborting reimport."); // Maybe handle in reimport of each importer
																			// to allow non file uris reimporting
		return;
	}

	if (auto it = m_extToImporters.find(fileExt); it != m_extToImporters.end()) {
		LOG_INFO("Execute reimporting: {} for {}", it->second->GetName(), fileExt);
		it->second->Reimport(entry, importUri);
	}
}

BasePodHandle ImporterRegistry::ImportImpl(const fs::path& path, mti::TypeId& outHandleType)
{
	outHandleType = mti::TypeId{};
	if (path.empty()) {
		return {};
	}
	//
	if (!fs::exists(path) && path.extension().compare(".shader") != 0) {
		LOG_WARN("Failed to find file: {} during import. Using default pod.", path);
		return {};
	}
	if (auto it = m_extToImporters.find(path.extension().string()); it != m_extToImporters.end()) {
		LOG_INFO("Executing importer: {} for {}", it->second->GetName(), path.generic_string());
		auto handle = it->second->Import(path);
		outHandleType = it->second->GetPrimaryPodType();
		return handle;
	}
	return {};
}

bool ImporterRegistry::ImportFile(const fs::path& path)
{
	mti::TypeId typeId;
	auto handle = ImportImpl(path, typeId);
	return !handle.IsDefault();
}
