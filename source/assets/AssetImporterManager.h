#pragma once
#include "assets/AssetRegistry.h"
#include "assets/ImporterRegistry.h"


inline class AssetImporterManager_ {
	friend class AssetManager_;
	friend class AssetHandlerManager;

	ImporterRegistry m_importerRegistry;


	// Stores this session's known imported paths.
	std::unordered_map<uri::Uri, BasePodHandle> m_importedPathsCache;


	std::vector<fs::path> m_pathsStack{ "gen-data/" };


	[[nodiscard]] uri::Uri GeneratePath(uri::Uri importPath, uri::Uri name)
	{
		auto& directory = m_pathsStack.back();
		auto path = AssetHandlerManager::SuggestPath((m_pathsStack.back() / name).generic_string());
		return path;
	}

public:
	//
	// Handling of paths we import to.
	//
	void PushPath(const fs::path& path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }
	void PushPath(std::string_view path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }
	void PushPath(const std::string& path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }

	// Overwrites all current paths and creates a new one relative from the root directory. Requires PopPath as the rest
	// PushPath functions
	void SetPushPath(const std::string& path) { m_pathsStack.emplace_back(path); }


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
		return CreateEntryFromImportImpl<PodType>(name, name, true, false, false);
	}

	template<CONC(CAssetPod) PodType>
	std::pair<PodHandle<PodType>, PodType*> CreateTransientEntryFromFile(
		const uri::Uri& name, const uri::Uri& importPath)
	{
		return CreateEntryFromImportImpl<PodType>(importPath, name, true, false, false);
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
		return CreateEntryFromImportImpl<PodType>(importPath, name, false, reimportOnLoad, exportOnSave);
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

private:
	template<CONC(CAssetPod) PodType>
	[[nodiscard]] std::pair<PodHandle<PodType>, PodType*> CreateEntryFromImportImpl(
		const uri::Uri& importPath, const uri::Uri& name, bool transient, bool reimportOnLoad, bool exportOnSave)
	{
		auto& [entry, ptr] = AssetHandlerManager::CreateEntry<PodType>(
			transient ? AssetHandlerManager::SuggestPath(name) : GeneratePath(importPath, name), transient, importPath,
			reimportOnLoad, exportOnSave);


		PodHandle<PodType> handle{ entry->uid };

		m_importedPathsCache.emplace(importPath, handle);

		return std::make_pair(handle, ptr);
	}


} * ImporterManager{};
