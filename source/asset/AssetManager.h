#pragma once

#include "asset/AssetPod.h"
#include "asset/UriLibrary.h"
#include "asset/PodHandle.h"
#include "reflection/PodReflection.h"
#include "engine/Engine.h"
#include "engine/Logger.h"
#include "engine/console/ConsoleVariable.h"
#include "asset/ImporterRegistry.h"
#include "core/StringUtl.h"
#include <future>

// DOC: OUTDATED, Please update.
// ASSET URI:
// Documentation of Asset URI convention: (DOC: ASSETS After refactor is finished)
//
// 4 types of assets: (first letter of the URI categorizes the type)
//
// * Default assets uri: "~X". ---> where X goes the UID index of this type (for debugging ONLY).
//
// * Runtime generated assets uri: "#flatGenPath" ---> flatGenPath needs to be a flat path (Maybe these won't stay in
//     final design and use just interal uid and handles without paths)
//
// * External disk file paths. "C:/Absolute/Disk/Path/x.gltf" OR (decide design) "../../relative/directory/x.gltf" these
//     paths can access "any" filesystem path to import from the user computer. This type of path MAY use additional
//     JSON data at the end of the path to show sub assets. eg: "../model.gltf{"mat"="gold"}". This path is what is
//     saved at the metadata field: originalImportLocation
//
// * Internal pod assets use the old URI system starting with "/" but CANNOT use json data and MUST resolve to a file
//     asset with .pod or .json with proper header metadata.
//

struct PodDeleter {
	void operator()(AssetPod* p);
};

struct ImagePod;
struct SamplerPod;
struct ShaderPod;
struct StringPod;
class ReflClass;

struct PodEntry {
	struct UnitializedPod {
	};

	UniquePtr<AssetPod, PodDeleter> ptr{};
	TypeId type{ refl::GetId<UnitializedPod>() };
	size_t uid{ 0 };
	uri::Uri path{};

	std::string name{}; // TODO: ASSETS fix data duplication

	// Mark pods as transient ones when they are just used for importing (or are generated) and "file-like" operations
	// like save and reimport are not allowed on them. eg: GltfFilePod, default constructed empty pods
	bool transient{ true };

	// Only usefull if transient is false
	bool requiresSave{ false };

	// Only usefull if transient is false
	PodMetaData metadata;


	template<typename T>
	T* UnsafeGet()
	{
		static_assert(
			std::is_base_of_v<AssetPod, T> && !std::is_same_v<AssetPod, T>, "Unsafe get called without a pod type");
		return static_cast<T*>(ptr.get());
	}

	template<CONC(CAssetPod) T>
	PodHandle<T> GetHandleAs()
	{
		CLOG_ABORT(type != mti::GetTypeId<T>(), "Entry->GetAs() Cast failure");
		return PodHandle<T>{ uid };
	}

	void MarkSave() { requiresSave = true; }

	// Prefer this from .name; .name will get deprecated in the future
	[[nodiscard]] std::string_view GetName() const { return uri::GetFilenameNoExt(path); }

	[[nodiscard]] std::string GetNameStr() const { return std::string(uri::GetFilenameNoExt(path)); }

	//
	// Refl Class Section (optimisation to avoid GetClass for each pod)
	//
	[[nodiscard]] const ReflClass* GetClass() const { return podClass; };

	// Used by importers and serializers
	[[nodiscard]] void Z_AssignClass(const ReflClass* cl) { podClass = cl; }

private:
	const ReflClass* podClass{ nullptr };
};

template<typename T>
size_t ToAssetUid(T t)
{
	return t;
};

template<>
inline size_t ToAssetUid<size_t>(size_t t)
{
	return t;
}

template<>
inline size_t ToAssetUid<BasePodHandle>(BasePodHandle handle)
{
	return handle.podId;
}

template<>
inline size_t ToAssetUid<PodEntry*>(PodEntry* entry)
{
	return entry->uid;
}

template<typename T>
concept CUidConvertible = requires(T a)
{
	{
		ToAssetUid(a)
	}
	->std::convertible_to<size_t>;
};

class AssetHandlerManager {

private:
	friend class AssetImporterManager;
	friend class AssetFrontEndManager;

	static AssetHandlerManager& Get()
	{
		static AssetHandlerManager inst = AssetHandlerManager();
		return inst;
	}

	AssetHandlerManager() {}

	std::vector<UniquePtr<PodEntry>> m_pods;
	std::unordered_map<uri::Uri, size_t, str::HashInsensitive> m_pathCache;


	uri::Uri SuggestFilenameImpl(std::string_view directory, const uri::Uri& desired)
	{
		uri::Uri desiredFilename = desired;

		auto dotLoc = desiredFilename.rfind('.');

		if (dotLoc != std::string::npos) {
			std::replace(desiredFilename.begin(), desiredFilename.begin() + dotLoc - 1, '.', '_');
		}

		auto desiredFullPath = (fs::path(directory) / desiredFilename).string();

		if (!m_pathCache.count(desiredFullPath)) {
			return desiredFilename;
		}

		std::string resultFilename = fmt::format(
			"{}_{}{}", uri::StripExt(desiredFilename), std::rand(), uri::GetDiskExtension(desiredFilename));
		return fs::path(desiredFilename).replace_filename(resultFilename).generic_string();
	}


	void SaveToDiskInternal(PodEntry* entry);
	void LoadAllPodsInDirectory(const fs::path& path);
	void LoadFromDiskTypelessInternal(PodEntry* entry);

	template<CONC(CAssetPod) T>
	PodHandle<T> GetAsyncHandleInternal(const uri::Uri& str)
	{
		auto it = m_pathCache.find(str);
		if (it == m_pathCache.end()) {
			return PodHandle<T>();
		}

		return PodHandle<T>(it->second);
	}

public:
	static void RegisterPathCache(PodEntry* entry) { Get().m_pathCache.emplace(entry->path, entry->uid); }

	static void SaveAll()
	{
		for (auto& entry : Get().m_pods) {
			SaveToDisk(entry.get());
		}
	}

	template<CONC(CAssetPod) T>
	static PodHandle<T> GetAsyncHandle(const uri::Uri& str)
	{
		return Get().GetAsyncHandleInternal<T>(str);
	}

	static uri::Uri GetPodImportPath(BasePodHandle handle)
	{
		return Get().m_pods[handle.podId]->metadata.originalImportLocation;
	}

	static void RemoveEntry(size_t uid) { Get().m_pods[uid].reset(); }

	static void RemoveEntry(BasePodHandle handle) { RemoveEntry(handle.podId); }

	static uri::Uri GetPodUri(BasePodHandle handle) { return Get().m_pods[handle.podId]->path; }

	static PodEntry* GetEntry(BasePodHandle handle) { return Get().m_pods[handle.podId].get(); }


	template<CONC(CUidConvertible) T>
	static void SaveToDisk(T asset)
	{
		size_t uid = ToAssetUid(asset);
		Get().SaveToDiskInternal(Get().m_pods[uid].get());
	}


	template<CONC(CUidConvertible) T>
	static void LoadFromDiskTypeless(T asset)
	{
		size_t uid = ToAssetUid(asset);
		Get().LoadFromDiskTypelessInternal(Get().m_pods[uid].get());
	}


	// Returns an alternative "valid" path for this asset. ie one that will not collide with a current asset.
	// If the passed in path is corret it will be returned instead
	static uri::Uri SuggestFilename(std::string_view directory, const uri::Uri& desiredFilename)
	{
		return Get().SuggestFilenameImpl(directory, desiredFilename);
	}

	template<CONC(CAssetPod) PodType>
	static PodType* Z_Handle_AccessPod(size_t podId)
	{
		return static_cast<PodType*>(Get().m_pods[podId]->ptr.get());
	}

	// AVOID THIS. This is for internal use.
	[[nodiscard]] static std::vector<UniquePtr<PodEntry>>& Z_GetPods() { return Get().m_pods; }
};


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
		auto directory = m_defaultImportingPath.string(); // WIP: Use importing operation
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


class AssetFrontEndManager {


public:
	inline static ImporterRegsitry importerRegistry{};
	// Used to determine whether to use importer vs async handles until the assets get a proper front-end for
	// importing
	template<CONC(CAssetPod) PodType>
	static PodHandle<PodType> TransitionalLoadAsset(const uri::Uri& str)
	{
		if (str.starts_with('/')) {
			return AssetFrontEndManager::ImportAs<PodType>(fs::path(str.substr(1)));
		}
		return AssetHandlerManager::GetAsyncHandle<PodType>(str);
	}

	static void Import(const fs::path& path) { importerRegistry.ImportFile(path); }

	template<CONC(CAssetPod) T>
	static PodHandle<T> ImportAs(const fs::path& path)
	{
		return importerRegistry.ImportFile<T>(path);
	}
};
