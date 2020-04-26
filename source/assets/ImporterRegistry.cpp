#include "pch.h"
#include "ImporterRegistry.h"

#include "assets/importers/GltfImporter.h"
#include "assets/importers/ImageImporter.h"
#include "assets/importers/ShaderImporter.h"
#include "assets/importers/CubemapImporter.h"

ImporterRegistry::ImporterRegistry()
{
	RegisterImporters<ImageImporter, GltfImporter, ShaderImporter, CubemapImporter>();
}

BasePodHandle ImporterRegistry::ImportImpl(const fs::path& path, mti::TypeId& outHandleType)
{
	outHandleType = mti::TypeId{};
	// DOC: New specification. Pod Importers should not have to check for path existance.
	if (!fs::exists(path)) {
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
	return handle.HasBeenAssigned();
}
