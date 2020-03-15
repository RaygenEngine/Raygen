#include "pch.h"
#include "asset/ImporterRegistry.h"

#include "asset/importers/ImageImporter.h"
#include "asset/importers/GltfImporter.h"
#include "asset/importers/ShaderImporter.h"


ImporterRegsitry::ImporterRegsitry()
{
	RegisterImporters<ImageImporter, GltfImporter, ShaderImporter>();
}

BasePodHandle ImporterRegsitry::ImportImpl(const fs::path& path, mti::TypeId& outHandleType)
{
	outHandleType = mti::TypeId{};
	if (auto it = m_extToImporters.find(path.extension().string()); it != m_extToImporters.end()) {
		auto handle = it->second->Import(path);
		outHandleType = it->second->GetPrimaryPodType();
		return handle;
	}
	return {};
}

bool ImporterRegsitry::ImportFile(const fs::path& path)
{
	mti::TypeId typeId;
	auto handle = ImportImpl(path, typeId);
	return handle.HasBeenAssigned();
}
