#pragma once
#include "asset/AssetHandlerManager.h"

// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetImporterManager {
public:
	void Init(const fs::path& assetPath);

private:
	// Stores this session's known imported paths.
	std::unordered_map<uri::Uri, BasePodHandle> m_importedPathsCache;

	fs::path m_defaultImportingPath{ "gen-data/" };

	struct ImportOperation {
		int32 stackSize{ 0 };
		fs::path currentOpImportingPath{};

		void PushOperation(AssetImporterManager& importer, fs::path& inOutImportingPath)
		{
			if (stackSize == 0) {
				if (inOutImportingPath.empty()) {
					// This warning happens when there is an  attempt to import an asset but an operation has not
					// started. This means that the imported assets will go to a "random" folder in the assets and not
					// the user selected one. Technically importing should only ever be done from the asset window or
					// other editor UI that allows the user to select where to import.
					LOG_WARN("Importing assets with default import path! (Manual import from code?)");

					currentOpImportingPath = importer.m_defaultImportingPath;
				}
				else {
					currentOpImportingPath = inOutImportingPath;
				}
			}

			stackSize++;
			if (inOutImportingPath.empty()) {
				inOutImportingPath = currentOpImportingPath;
			}
		}

		void PopOperation() { stackSize--; }
	};

	ImportOperation m_currentOperation;


private:
	uri::Uri GeneratePath(uri::Uri importPath, uri::Uri name)
	{
		auto directory = m_defaultImportingPath.string(); // NEXT: Use importing operation
		auto filename = AssetHandlerManager::SuggestFilename(directory, name);
		return directory + filename;
	}

public:
	template<CONC(CAssetPod) PodType>
	static std::pair<PodHandle<PodType>, PodType*> CreateTransientEntry(const uri::Uri& name)
	{
		return CreateEntryImpl<PodType>(name, name, true, false, false);
	}

	//
	// Interface below designed for Pod Importers
	// contains utilities for entry registrations and calling other importers.
	// All functions in this section expect to be called from the body of a PodImporter's import function
	//


	template<CONC(CAssetPod) PodType>
	static std::pair<PodHandle<PodType>, PodType*> CreateEntry(
		const uri::Uri& importPath, const uri::Uri& name, bool reimportOnLoad = false, bool exportOnSave = false)
	{
		return CreateEntryImpl<PodType>(importPath, name, false, reimportOnLoad, exportOnSave);
	}


	template<CONC(CAssetPod) T>
	static PodHandle<T> ImportRequest(const fs::path& path)
	{
		auto inst = Engine.GetAssetImporterManager();
		auto it = inst->m_importedPathsCache.find(path.generic_string());

		if (it != inst->m_importedPathsCache.end()) {
			return it->second;
		}

		return AssetFrontEndManager::importerRegistry.ImportFile<T>(path);
	}

private:
	template<CONC(CAssetPod) PodType>
	static std::pair<PodHandle<PodType>, PodType*> CreateEntryImpl(
		const uri::Uri& importPath, const uri::Uri& name, bool transient, bool reimportOnLoad, bool exportOnSave)
	{
		auto inst = Engine.GetAssetImporterManager();
		PodEntry* e = new PodEntry();

		// Populate Metadata
		e->metadata.originalImportLocation = importPath;
		e->metadata.reimportOnLoad = reimportOnLoad;
		e->metadata.exportOnSave = exportOnSave;
		e->metadata.podTypeHash = mti::GetHash<PodType>();

		// Populate entry data
		e->requiresSave = true;

		auto ptr = new PodType();
		e->ptr.reset(ptr);
		e->transient = transient;
		e->Z_AssignClass(&PodType::StaticClass());
		e->type = mti::GetTypeId<PodType>();


		e->path = transient ? AssetHandlerManager::SuggestFilename("", name) : inst->GeneratePath(importPath, name);
		e->name = uri::GetFilename(e->path);


		e->uid = AssetHandlerManager::Get().m_pods.size();
		AssetHandlerManager::Get().m_pods.emplace_back(e);
		AssetHandlerManager::RegisterPathCache(e);


		PodHandle<PodType> handle{ e->uid };

		inst->m_importedPathsCache.emplace(importPath, handle);

		return std::make_pair(handle, ptr);
	}
};
