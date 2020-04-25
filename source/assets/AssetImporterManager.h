#pragma once
#include "assets/AssetRegistry.h"
#include "assets/ImporterRegistry.h"


inline class AssetImporterManager_ {
	friend class AssetManager_;
	ImporterRegistry m_importerRegistry;


	// Stores this session's known imported paths.
	std::unordered_map<uri::Uri, BasePodHandle> m_importedPathsCache;


	std::vector<fs::path> m_pathsStack{ "gen-data/" };


	[[nodiscard]] uri::Uri GeneratePath(uri::Uri importPath, uri::Uri name)
	{
		auto& directory = m_pathsStack.back();
		auto filename = AssetHandlerManager::SuggestFilename(m_pathsStack.back(), name);
		return fs::path(directory / filename).generic_string();
	}

public:
	//
	// Handling of paths we import to.
	//
	void PushPath(const fs::path& path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }
	void PushPath(std::string_view path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }
	void PushPath(const std::string& path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }
	void PushPath(const char* path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }

	void PopPath()
	{
		CLOG_ERROR(m_pathsStack.size() == 1, "More pops than pushes for importing path found!");
		if (m_pathsStack.size() > 1) {
			m_pathsStack.pop_back();
		}
	}
	//

	template<CONC(CAssetPod) PodType>
	std::pair<PodHandle<PodType>, PodType*> CreateTransientEntry(const uri::Uri& name)
	{
		return CreateEntryImpl<PodType>(name, name, true, false, false);
	}

	template<CONC(CAssetPod) PodType>
	std::pair<PodHandle<PodType>, PodType*> CreateTransientEntryFromFile(
		const uri::Uri& name, const uri::Uri& importPath)
	{
		return CreateEntryImpl<PodType>(importPath, name, true, false, false);
	}

	//
	// Interface below designed for Pod Importers
	// contains utilities for entry registrations and calling other importers.
	// All functions in this section expect to be called from the body of a PodImporter's import function
	//


	template<CONC(CAssetPod) PodType>
	[[nodiscard]] std::pair<PodHandle<PodType>, PodType*> CreateEntry(
		const uri::Uri& importPath, const uri::Uri& name, bool reimportOnLoad = false, bool exportOnSave = false)
	{
		return CreateEntryImpl<PodType>(importPath, name, false, reimportOnLoad, exportOnSave);
	}


	template<CONC(CAssetPod) T>
	[[nodiscard]] PodHandle<T> ImportRequest(const fs::path& path)
	{
		auto it = m_importedPathsCache.find(path.generic_string());

		if (it != m_importedPathsCache.end()) {
			return it->second;
		}
		size_t stack = m_pathsStack.size();
		auto handle = m_importerRegistry.ImportFile<T>(path);

		CLOG_ABORT(m_pathsStack.size() != stack,
			"Stack missmatch at importing directory, did you forget to pop stack in your importer?");

		return handle;
	}

	// Returns a mutable pod handle that should be used temporarily but not stored in objects.
	// This function will NOT always generate a new pod but may use the old inoutHandleRef pod.
	// example usage:
	//
	// auto editablePod = CreateGeneratedPod(MySkybox->m_generatedSkyboxHandle, false);
	//
	template<CONC(CAssetPod) T>
	MutablePod<T> CreateGeneratedPod(PodHandle<T>& inoutHandleRef, bool transient)
	{
		auto entry = AssetHandlerManager::GetEntry(inoutHandleRef.uid);
		if (entry->metadata.generated) {
			return MutablePod<T>{ inoutHandleRef.uid };
		}
		inoutHandleRef
			= CreateEntryImpl<T>("#", fmt::format("#_{}_{}", mti::GetName<T>(), rand()), transient, false, false, true)
				  .first;
		return MutablePod<T>{ inoutHandleRef.uid };
	}

private:
	template<CONC(CAssetPod) PodType>
	[[nodiscard]] std::pair<PodHandle<PodType>, PodType*> CreateEntryImpl(const uri::Uri& importPath,
		const uri::Uri& name, bool transient, bool reimportOnLoad, bool exportOnSave, bool generatedPod = false)
	{
		PodEntry* e = new PodEntry();

		// Populate Metadata

		e->metadata.originalImportLocation = generatedPod ? "#" : importPath;
		e->metadata.reimportOnLoad = reimportOnLoad;
		e->metadata.generated = generatedPod;
		e->metadata.exportOnSave = exportOnSave;
		e->metadata.podTypeHash = mti::GetHash<PodType>();

		// Populate entry data
		e->requiresSave = !transient;

		auto ptr = new PodType();
		e->ptr.reset(ptr);
		e->transient = transient;
		e->Z_AssignClass(&PodType::StaticClass());
		e->type = mti::GetTypeId<PodType>();


		if (transient) {
			e->path = AssetHandlerManager::SuggestFilename("", name);
		}
		else if (generatedPod) {
			e->path = "gen-data/level-data/" + AssetHandlerManager::SuggestFilename("gen-data/level-data", name);
		}
		else {
			e->path = GeneratePath(importPath, name);
		}


		e->name = uri::GetFilename(e->path);


		e->uid = AssetHandlerManager::Get().m_pods.size();
		AssetHandlerManager::Get().m_pods.emplace_back(e);
		AssetHandlerManager::RegisterPathCache(e);


		PodHandle<PodType> handle{ e->uid };

		if (!generatedPod) {
			m_importedPathsCache.emplace(importPath, handle);
		}

		return std::make_pair(handle, ptr);
	}
} * ImporterManager{};
