#pragma once

#include "asset/AssetPod.h"
#include "asset/UriLibrary.h"
#include "asset/PodHandle.h"
#include "reflection/PodReflection.h"
#include "system/Engine.h"
#include "system/Logger.h"
#include "system/console/ConsoleVariable.h"

#include <future>

// ASSET URI:
// Documentation of Asset URI convention: (WIP: ASSETS After refactor is finished)
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
struct TexturePod;
struct ShaderPod;
struct StringPod;
class ReflClass;

struct PodEntry {
	struct UnitializedPod {
	};

	std::unique_ptr<AssetPod, PodDeleter> ptr{};
	TypeId type{ refl::GetId<UnitializedPod>() };
	size_t uid{ 0 };
	uri::Uri path{};

	std::string name{}; // TODO: fix data duplication

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

	std::vector<std::unique_ptr<PodEntry>> m_pods;
	std::unordered_map<uri::Uri, size_t> m_pathCache;


	uri::Uri SuggestFilenameImpl(std::string_view directory, const uri::Uri& desired)
	{
		uri::Uri desiredFilename = desired;

		auto dotLoc = desiredFilename.rfind('.');

		if (dotLoc != std::string::npos) {
			std::replace(desiredFilename.begin(), desiredFilename.begin() + dotLoc - 1, '.', '_');
		}

		auto desiredFullPath = (fs::path(directory) / uri::StripExt(desiredFilename)).string();

		// WIP: ASSETS implement with hashmaps and/or internal directory format
		// THIS IS ULTRA SLOW N^2
		for (auto& pod : m_pods) {
			if (uri::StripExt(pod->path) == desiredFullPath) {
				std::string resultFilename = fmt::format(
					"{}_{}{}", uri::StripExt(desiredFilename), std::rand(), uri::GetDiskExtension(desiredFilename));
				return fs::path(desiredFilename).replace_filename(resultFilename).generic_string();
			}
		}

		return desiredFilename;
	}


	void SaveToDiskInternal(PodEntry* entry);
	void LoadAllPodsInDirectory(const fs::path& path);
	void LoadFromDiskTypelesskInternal(PodEntry* entry);

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

	template<CONC(CAssetPod) PodT>
	static PodEntry* CreateNew()
	{
		// Add entry here. Return entry to allow edits / w.e else.
		// Whoever "Creates" the asset should be responsible for filling in the metadat
		// (feels like a less decoupled instead of having multple CreateNewFromDisk etc)

		auto entry = CreateNewTypeless();
		entry->type = mti::GetTypeId<PodT>();
		entry->metadata.podTypeHash = entry->type.hash();
		entry->Z_AssignClass(&PodT::StaticClass());
		return entry;
	}

	// Initialized fields:
	// entry->uid
	static PodEntry* CreateNewTypeless()
	{
		auto entryUnq = std::make_unique<PodEntry>();
		auto entry = entryUnq.get();
		entry->uid = Get().m_pods.size();

		Get().m_pods.push_back(std::move(entryUnq));

		return entry;
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
		Get().LoadFromDiskTypelesskInternal(Get().m_pods[uid].get());
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
	// TODO: private and friend this
	[[nodiscard]] static std::vector<std::unique_ptr<PodEntry>>& Z_GetPods() { return Get().m_pods; }
};


namespace ed {
class AssetsWindow;
}
// asset cache responsible for "cpu" files (xmd, images, string files, xml files, etc)
class AssetImporterManager {
public:
	void Init(const fs::path& assetPath);

private:
	// Stores this session's known imported paths. Any ResolveOrImport that hits this just returns the handle
	std::unordered_map<uri::Uri, BasePodHandle> m_importedPathsCache;

	fs::path m_defaultImportingPath{ "gen-data/" };

	struct ImportOperation {
		int32 stackSize{ 0 };
		fs::path currentOpImportingPath{};

		void PushOperation(AssetImporterManager& importer, fs::path& inOutImportingPath)
		{
			if (stackSize == 0) {
				if (inOutImportingPath.empty()) {
					// This warning happens when an arbitiary (as in: Not from importers) ResolveOrImport attempts to
					// import an asset but is not given an importingPath argument. This means that the imported assets
					// will go to a "random" folder in the assets and not the user selected one. Technically importing
					// should only ever be done from the asset window or other editor UI that allows the user to select
					// where to import.
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

public:
	//
	// PUBLIC ASSET IMPORTING INTERFACE
	//

	// Search in main AssetHandlerManager import paths to find this as an absolute path
	// if found return it,
	// otherwise load it right away and forward it in AssetHandlerManager
	// NOTE: this can make infinite recursion on circlar dependencies, do we handle this? (possible, needs extra
	// code)
	// NOTE: We only cache since the execution of the program
	template<CONC(CAssetPod) PodType>
	static PodHandle<PodType> ResolveOrImport(
		const fs::path& inFullPath, const uri::Uri& suggestedName = "", fs::path& importingPath = fs::path())
	{
		auto inst = Engine::GetAssetImporterManager();


		auto [it, didInsert] = inst->m_importedPathsCache.try_emplace(inFullPath.string(), PodHandle<PodType>{});

		if (!didInsert) {
			// TODO: type check this handle
			return PodHandle<PodType>(it->second);
		}

		inst->m_currentOperation.PushOperation(*inst, importingPath);
		PodHandle<PodType> handle = inst->ImportFromDisk<PodType>(inFullPath, suggestedName, importingPath);
		inst->m_currentOperation.PopOperation();
		it->second = handle;
		return handle;
	}

	template<CONC(CAssetPod) PodType>
	static PodHandle<PodType> ResolveOrImportFromParentUri(const fs::path& path, const uri::Uri& parentUri,
		const uri::Uri& suggestedName = "", fs::path& importingPath = fs::path())
	{
		if (parentUri.size() <= 1) {
			return AssetImporterManager::ResolveOrImport<PodType>(path, suggestedName, importingPath);
		}

		auto dskPath = uri::GetDiskPath(parentUri);
		auto parentDir = uri::GetDir(dskPath);             // Get parent directory. (Also removes json)
		fs::path resolvedUri = fs::path(parentDir) / path; // add path (path may include json data at the end)

		return AssetImporterManager::ResolveOrImport<PodType>(resolvedUri, suggestedName, importingPath);
	}

	template<CONC(CAssetPod) PodType>
	static PodHandle<PodType> ResolveOrImportFromParent(const fs::path& path, BasePodHandle parentHandle,
		const uri::Uri& suggestedName = "", fs::path& importingPath = fs::path())
	{
		CLOG_ABORT(path.empty(), "Path was empty. Parent was: {}", AssetHandlerManager::GetPodImportPath(parentHandle));
		return ResolveOrImportFromParentUri<PodType>(
			path, AssetHandlerManager::GetPodImportPath(parentHandle), suggestedName, importingPath);
	}


private:
	ConsoleVariable<bool> m_longImportPaths{ "a.longImportPaths", false };

	template<CONC(CAssetPod) PodType>
	PodHandle<PodType> ImportFromDisk(
		const fs::path& path, const uri::Uri& suggestedName, const fs::path& importingPath)
	{
		PodEntry* entry = AssetHandlerManager::CreateNew<PodType>();
		entry->metadata.originalImportLocation = path.generic_string();
		entry->name = suggestedName;
		if (!TryImport<PodType>(entry, importingPath)) {
			AssetHandlerManager::RemoveEntry(entry->uid);
			return PodHandle<PodType>(); // Will return the proper "default" pod of this type
		}
		return PodHandle<PodType>{ entry->uid };
	}

	template<CONC(CAssetPod) T>
	bool TryImport(PodEntry* entry, const fs::path& importingPath)
	{
		if (entry->metadata.originalImportLocation.empty()) {
			LOG_ERROR("Failed to import. No asset location");
			return false;
		}

		if (entry->type != mti::GetTypeId<T>() || entry->metadata.podTypeHash != mti::GetHash<T>()) {
			LOG_ERROR("Failed to import. Type missmatch");
			return false;
		}

		//
		// Calling the actual import function
		//

		auto ptr = new T();
		entry->ptr.reset(ptr);
		entry->transient = false; // All entries that are "imported" default as non-transient. Importers can change this
								  // value during importing
		try {
			T::Load(entry, ptr, entry->metadata.originalImportLocation);
		} catch (std::exception& e) {
			LOG_ERROR("Failed to import: {} {} Exception:\n{}\nAll dependent import assets may be incorrect.",
				refl::GetName<T>(), entry->metadata.originalImportLocation, e.what());
			return false;
		}


		//
		// Post import guarantees (all fields must be initialized)
		//
		// WIP: ASSETS: some stuff should be moved in import functions because it depends on specific loaders + asset
		// types
		auto interm = (importingPath / uri::GetFilename(entry->metadata.originalImportLocation));
		auto fullRelativePath = interm.relative_path().generic_string();

		bool generatedName{ false };
		if (entry->name.empty()) {
			if (uri::HasJson(entry->metadata.originalImportLocation)) {
				entry->name = fmt::format("{}_{}", uri::GetFilenameNoExt(entry->metadata.originalImportLocation),
					mti::GetName<T>(), std::rand());
			}
			else {
				entry->name = uri::GetFilenameNoExt(fullRelativePath);
			}
		}


		auto ext = entry->metadata.preferedDiskType == PodDiskType::Binary ? "bin" : "json";

		auto name = fmt::format("{}.{}", fs::path(entry->name).filename().string(), ext);
		entry->name = AssetHandlerManager::SuggestFilename(uri::GetDir(fullRelativePath), name);

		entry->path = fs::path(uri::GetDiskPath(fullRelativePath)).replace_filename(entry->name).string();

		LOG_INFO("Imported {:<12}: {}", entry->type.name(), entry->path);
		entry->MarkSave();
		return true;
	}
};


class AssetFrontEndManager {

public:
	// Used to determine whether to use importer vs async handles until the assets get a proper front-end for importing
	template<CONC(CAssetPod) PodType>
	static PodHandle<PodType> TransitionalLoadAsset(const uri::Uri& str)
	{
		if (str.starts_with('/')) {
			return AssetImporterManager::ResolveOrImport<PodType>(str);
		}
		return AssetHandlerManager::GetAsyncHandle<PodType>(str);
	}
};
