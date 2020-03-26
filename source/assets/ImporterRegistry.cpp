#include "pch.h"
#include "assets/ImporterRegistry.h"

#include "assets/importers/GltfImporter.h"
#include "assets/importers/ImageImporter.h"
#include "assets/importers/ShaderImporter.h"

ImporterRegistry::ImporterRegistry()
{
	RegisterImporters<ImageImporter, GltfImporter, ShaderImporter>();
}

BasePodHandle ImporterRegistry::ImportImpl(const fs::path& path, mti::TypeId& outHandleType)
{
	outHandleType = mti::TypeId{};
	if (auto it = m_extToImporters.find(path.extension().string()); it != m_extToImporters.end()) {
		LOG_REPORT("Executing importer: {} for {}", it->second->GetName(), path.generic_string());
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
