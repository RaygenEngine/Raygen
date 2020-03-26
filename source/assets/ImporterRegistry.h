#pragma once
#include "assets/importers/PodImporter.h"
#include "core/StringUtl.h"

#include <unordered_map>

class ImporterRegsitry {
	std::vector<UniquePtr<PodImporterBase>> m_importers;

	std::unordered_map<std::string, PodImporterBase*, str::HashInsensitive> m_extToImporters;


public:
	ImporterRegsitry();

	bool ImportFile(const fs::path& path);


	template<typename T>
	PodHandle<T> ImportFile(const fs::path& path)
	{
		mti::TypeId type{};
		BasePodHandle baseHandle = ImportImpl(path, type);
		if (type == mti::GetTypeId<T>()) {
			return PodHandle<T>{ baseHandle.podId };
		}
		return {};
	}

private:
	BasePodHandle ImportImpl(const fs::path& path, mti::TypeId& outHandleType);

	template<typename T>
	void RegisterImporter()
	{
		PodImporterBase* importer = m_importers.emplace_back(std::make_unique<T>(mti::GetName<T>())).get();

		for (auto ext : importer->GetSupportedExtensions()) {
			m_extToImporters.emplace(std::string(ext), importer);
		}
	}


	template<typename... T>
	void RegisterImporters()
	{
		(RegisterImporter<T>(), ...);
	}
};
