#pragma once
#include "assets/AssetImporterManager.h"
#include "assets/AssetRegistry.h"
#include "assets/ImporterRegistry.h"
#include "assets/UriLibrary.h"

#include <nlohmann/json.hpp>

inline AssetImporterManager_* AssetImporterManager{};

class AssetImporterManager_ {
	friend class AssetManager_;
	friend class AssetRegistry;

	ImporterRegistry m_importerRegistry;


	// Stores this session's known imported paths.
	std::unordered_map<uri::Uri, BasePodHandle> m_importedPathsCache;


	std::vector<fs::path> m_pathsStack{ "gen-data/" };


	[[nodiscard]] uri::Uri GeneratePath(uri::Uri importPath, uri::Uri name)
	{
		auto& directory = m_pathsStack.back();
		auto path = AssetRegistry::SuggestPath((m_pathsStack.back() / name).generic_string());
		return path;
	}

public:
	//
	// Handling of paths we import to.
	//
	void PushPath(const fs::path& path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }
	void PushPath(std::string_view path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }
	void PushPath(const std::string& path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }
	void PushPath(const char* path) { m_pathsStack.emplace_back(m_pathsStack.back() / path); }

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

	template<CAssetPod PodType>
	std::pair<PodHandle<PodType>, PodType*> CreateTransientEntry(const uri::Uri& name)
	{
		return CreateEntryFromImportImpl<PodType>(name, name, true, false, false);
	}

	template<CAssetPod PodType>
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


	template<CAssetPod PodType>
	[[nodiscard]] std::pair<PodHandle<PodType>, PodType*> CreateEntry(
		const uri::Uri& importPath, const uri::Uri& name, bool reimportOnLoad = false, bool exportOnSave = false)
	{
		return CreateEntryFromImportImpl<PodType>(importPath, name, false, reimportOnLoad, exportOnSave);
	}


	template<CAssetPod T>
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

	// CHECK: Requires json include (non standard cpp)
	// Matches the json object generated from AssetHandlerManager::GenerateRelativeExportJsonObject
	template<CAssetPod T>
	PodHandle<T> ImportOrFindFromJson(
		const nlohmann::json& json, const fs::path& relativeFilePath = "", bool useBinaries = true)
	{
		std::string strpath;

		if (!json.is_object()) {
			LOG_ERROR("Import or find from json expects json object: {}", relativeFilePath.generic_string());
			return PodHandle<T>{};
		}

		PathReferenceType pathType{};
		auto tie = GenMetaEnum(pathType);


		auto type = json.begin().key();
		if (!tie.SetValueByStr(type)) {
			LOG_ERROR("Incorrect enum value for ImportOrFindFromJson, {}. Value was: {}", relativeFilePath,
				json.begin()->get<std::string>());
			return PodHandle<T>{};
		}

		auto path = json.begin().value().get<std::string>();

		return ImportFromMaybeRelative<T>(pathType, path, relativeFilePath, useBinaries);
	}

	// TODO: Implement useBinaries
	// When use binaries is true, the system will check the import path of binary files and avoid reimporting if a
	// matching binary file is found.
	template<CAssetPod T>
	PodHandle<T> ImportFromMaybeRelative(PathReferenceType pathType, const fs::path& path,
		const fs::path& relativeFilePath = "", bool useBinaries = true)
	{
		if (pathType == PathReferenceType::FullPath) {
			return AssetImporterManager->ImportRequest<T>(fs::absolute(path));
		}

		if (pathType == PathReferenceType::WorkingDir) {
			// NOTE: pass just the path because whatever passed here is written as the original import location (and
			// we want that to be as relative as possible)
			return AssetImporterManager->ImportRequest<T>(path);
		}


		if (pathType == PathReferenceType::FileRelative) {
			// TODO: If file not found, search as if pathType was WorkingDir
			if (relativeFilePath.empty()) {
				LOG_ERROR("File relative import had relative File Path empty! {}", path);
				return {};
			}
			fs::path searchPath;
			auto cwd = fs::current_path();

			auto relativePathNoFilename
				= relativeFilePath.has_filename() ? relativeFilePath.parent_path() : relativeFilePath;

			if (relativePathNoFilename.is_absolute() && !std::equal(cwd.begin(), cwd.end(), relativeFilePath.begin())) {
				searchPath = relativePathNoFilename / path;
			}
			else {
				searchPath = fs::relative(relativePathNoFilename) / path;
			}
			return AssetImporterManager->ImportRequest<T>(searchPath);
		}

		if (pathType == PathReferenceType::BinaryAsset) {
			auto handle = AssetRegistry::GetAsyncHandle<T>(path.generic_string());
			if (!handle.IsDefault()) {
				return handle;
			}
			return {};
		}

		LOG_ABORT("Unhandled enum case");
		return {};
	}


private:
	template<CAssetPod PodType>
	[[nodiscard]] std::pair<PodHandle<PodType>, PodType*> CreateEntryFromImportImpl(
		const uri::Uri& importPath, const uri::Uri& name, bool transient, bool reimportOnLoad, bool exportOnSave)
	{
		auto&& [entry, ptr] = AssetRegistry::CreateEntry<PodType>(
			transient ? AssetRegistry::SuggestPath(name) : GeneratePath(importPath, name), transient, importPath,
			reimportOnLoad, exportOnSave);


		PodHandle<PodType> handle{ entry->uid };

		m_importedPathsCache.emplace(importPath, handle);

		return std::make_pair(handle, ptr);
	}
};
