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
// Documentation of Asset URI convention: (WIP: After refactor is finished)
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


struct PodEntry {
	struct UnitializedPod {
	};

	std::unique_ptr<AssetPod, PodDeleter> ptr{};
	TypeId type{ refl::GetId<UnitializedPod>() };
	size_t uid{ 0 };
	uri::Uri path{};
	// WIP: remove name, just get it from path with new assets
	std::string name{};

	// WIP: optional, only our custom disk assets
	PodMetaData metadata;

	bool requiresSave{ false };

	template<typename T>
	T* UnsafeGet()
	{
		static_assert(
			std::is_base_of_v<AssetPod, T> && !std::is_same_v<AssetPod, T>, "Unsafe get called without a pod type");
		return static_cast<T*>(ptr.get());
	}

	void MarkSave() { requiresSave = true; }
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

	ConsoleFunction<> m_SaveAll{ "SaveAll" };

public:
	friend class AssetImporterManager;
	friend class AssetFrontEndManager;

	static AssetHandlerManager& Get()
	{
		static AssetHandlerManager inst = AssetHandlerManager();
		return inst;
	}

	AssetHandlerManager()
	{
		m_SaveAll.function = [&]() {
			for (auto& entry : m_pods) {
				SaveToDisk(entry.get());
			}
		};
	}

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

		// WIP: implement with hashmaps and/or internal directory format
		// THIS IS ULTRA SLOW N^2
		for (auto& pod : m_pods) {
			if (uri::StripExt(pod->path) == desiredFullPath) {
				std::string resultFilename = fmt::format(
					"{}_{}{}", uri::StripExt(desiredFilename), std::rand(), uri::GetDiskExtension(desiredFilename));
				return fs::path(desiredFilename).replace_filename(resultFilename).string();
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
		if (str.starts_with('/')) {
			return AssetImporterManager::ResolveOrImport<T>(str);
		}
		auto it = m_pathCache.find(str);
		if (it == m_pathCache.end()) {
			return PodHandle<T>();
		}

		return PodHandle<T>(it->second);
	}

public:
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
		// WIP
		// Add entry here. Return entry to allow edits / w.e else.
		// Whoever "Creates" the asset should be responsible for filling in the metadat
		// (feels like a less decoupled instead of having multple CreateNewFromDisk etc)
		return CreateNew(mti::GetTypeId<PodT>());
	}

	static PodEntry* CreateNew(TypeId podType)
	{
		auto entry = CreateNewTypeless();
		entry->type = podType;
		entry->metadata.podTypeHash = podType.hash();
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

	fs::path m_currentImportingPath{ "gen-data/" };

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
		if (importingPath.empty()) {
			importingPath = inst->m_currentImportingPath;
		}


		auto [it, didInsert] = inst->m_importedPathsCache.try_emplace(inFullPath.string(), PodHandle<PodType>{});

		if (!didInsert) {
			// TODO: type check this handle
			return PodHandle<PodType>(it->second);
		}

		PodHandle<PodType> handle = inst->ImportFromDisk<PodType>(inFullPath, suggestedName, importingPath);
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
	template<CONC(CAssetPod) PodType>
	PodHandle<PodType> ImportFromDisk(
		const fs::path& path, const uri::Uri& suggestedName, const fs::path& importingPath)
	{
		PodEntry* entry = AssetHandlerManager::CreateNew<PodType>();
		entry->metadata.originalImportLocation = path.string();
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
		// WIP: some stuff should be moved in import functions because it depends on specific loaders + asset types

		// WIP: currently only works for paths relative to assets/
		// needs to be able to import from anywhere, into the root of importingPathing
		auto dskImportPath = fs::path(uri::ToSystemPath(uri::GetDiskPath(entry->metadata.originalImportLocation)));
		auto interm = (importingPath / dskImportPath);
		auto fullRelativePath = interm.relative_path().string();

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
};
