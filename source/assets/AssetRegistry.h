#pragma once
#include "assets/AssetPod.h"
#include "assets/AssetUpdateInfo.h"
#include "assets/PodEntry.h"
#include "assets/PodHandle.h"
#include "assets/UriLibrary.h"
#include "core/StringUtl.h"


struct Image;
struct Sampler;
struct Shader;
struct StringPod;
class ReflClass;


//
// Export path specification (Applied when exporting an asset):
//
// In General: Use the shortest reference point.
// The cases: (A: Directory of file currently exported, B: Directory of dependant file reference)
//
// In the examples all paths are full paths.
//
// 1. A == B: Exporting to the same directory, Written: FileRelative, "reference.exp"
//
// 2. current_path == "C:/notxy/"
//    A: "C:/xy/export.exp"
//    B: "C:/xy/subfolder1/subfolder2/reference.exp"
//
//    (Note: B not under current path, otherwise see case 4)
//	  Exporting to parent folder, Written: FileRelative, "subfolder1/subfolder2/reference.exp"
//
// 3. current_path == "C:/raygen/assets/"
//	  A: "C:/raygen/assets/folder/sub1/export.exp"
//    B: "C:/raygen/assets/folder/sub2/reference.exp"
//
//    Both under current working directory, incompatible parent_path's,
//    Written: WorkingDir, "folder/sub2/reference.exp"
//
// 4. current_path == "C:/xy/raygen/assets/"
//	  A: "C:/xy/somewhere/export.exp"
//	  B: "C:/xy/raygen/assets/folder/reference.exp"
//
//    (Note: B IS under current path, otherwise see case 2)
//	  Exporting with dependency to engine asset, Written: WorkingDir, "folder/reference.exp"
//
// 5. A: "C:/dir/xy/export.exp"
//	  B: "C:/dir/zw/folder/reference.exp"
//
//    Completely unknown / irrelevant paths, export full path as last resort.
//    Written: FullPath, "C:/dir/zw/folder/reference.exp"
//


enum class PathReferenceType
{
	FullPath,
	WorkingDir,
	FileRelative,
	BinaryAsset
};

namespace detail {
PathReferenceType GenerateExportDependencyPath(
	const fs::path& exporteePath, const fs::path& dependencyPath, fs::path& outPath);
}

class AssetRegistry {

private:
	friend class AssetImporterManager_;
	friend class AssetManager_;

	static AssetRegistry& Get()
	{
		static AssetRegistry inst = AssetRegistry();
		return inst;
	}

	std::vector<UniquePtr<PodEntry>> m_pods;
	std::unordered_map<uri::Uri, size_t, str::HashInsensitive> m_pathCache;

	std::vector<std::pair<size_t, AssetUpdateInfo>> m_gpuPodUpdateRequests;

	uri::Uri SuggestPathImpl(const uri::Uri& desiredFullPath)
	{
		if (!m_pathCache.count(desiredFullPath)) {
			return desiredFullPath;
		}

		std::string_view actualName;
		size_t num = uri::DetectCountFromPath(desiredFullPath, actualName);

		std::string nameStart = std::string(actualName) + (num == 0 ? "_" : "");
		std::string nextName;

		do {
			nextName = fmt::format("{}{}", nameStart, ++num);
		} while (m_pathCache.count(nextName));

		return nextName;
	}


	void SaveToDiskInternal(PodEntry* entry);
	void LoadAllPodsInDirectory(const fs::path& path);
	void LoadFromDiskTypelessInternal(PodEntry* entry);
	void ReimportFromOriginalInternal(PodEntry* entry);
	void ExportToLocationImpl(PodEntry* entry, const fs::path& path);

	template<CAssetPod T>
	PodHandle<T> GetAsyncHandleInternal(const uri::Uri& str)
	{
		auto it = m_pathCache.find(str);
		if (it == m_pathCache.end()) {
			return PodHandle<T>();
		}

		return PodHandle<T>(it->second);
	}

	// For name collision.
	void RenameEntryImpl(PodEntry* entry, const std::string_view newFullPath);


	void DeleteFromDiskInternal(PodEntry* entry);

	PodEntry* DuplicateImpl(PodEntry* entry);

public:
	static void RegisterPathCache(PodEntry* entry) { Get().m_pathCache.emplace(entry->path, entry->uid); }
	static void RemoveFromPathCache(PodEntry* entry) { Get().m_pathCache.erase(entry->path); }

	// Exports the asset directly to the specified location without taking into account ANY metadata / asset flags.
	template<CUidConvertible T>
	static void ExportToLocation(T asset, const fs::path& diskLocation)
	{
		size_t uid = ToAssetUid(asset);
		return Get().ExportToLocationImpl(Get().m_pods[uid].get(), diskLocation);
	}

	// There is no fast version of this function, it is just O(N) for loaded assets doing string comparisons
	template<CAssetPod T>
	static PodHandle<T> SearchForAssetFromImportPathSlow(std::string_view endsWith)
	{
		for (auto& entry : Get().m_pods) {
			[[unlikely]] //
			if (entry->IsA<T>() && entry->metadata.originalImportLocation.ends_with(endsWith))
			{
				return entry->GetHandleAs<T>();
			}
		}
		return {};
	}

	static void SaveAll()
	{
		for (auto& entry : Get().m_pods) {
			SaveToDisk(entry.get());
		}
	}

	template<CAssetPod T>
	static PodHandle<T> GetAsyncHandle(const uri::Uri& str)
	{
		return Get().GetAsyncHandleInternal<T>(str);
	}

	static uri::Uri GetPodImportPath(BasePodHandle handle)
	{
		return Get().m_pods[handle.uid]->metadata.originalImportLocation;
	}


	template<CUidConvertible T>
	static PodEntry* Duplicate(T asset)
	{
		size_t uid = ToAssetUid(asset);
		return Get().DuplicateImpl(Get().m_pods[uid].get());
	}

	static void RemoveEntry(size_t uid) { Get().m_pods[uid].reset(); }

	static void RemoveEntry(BasePodHandle handle) { RemoveEntry(handle.uid); }

	static uri::Uri GetPodUri(BasePodHandle handle) { return Get().m_pods[handle.uid]->path; }


	template<CUidConvertible T>
	static PodEntry* GetEntry(T asset)
	{
		size_t uid = ToAssetUid(asset);
		return Get().m_pods[uid].get();
	}


	static PodEntry* GetEntry(BasePodHandle handle) { return Get().m_pods[handle.uid].get(); }

	static void RenameEntry(PodEntry* entry, const std::string_view newFullPath)
	{
		Get().RenameEntryImpl(entry, newFullPath);
	}

	template<CUidConvertible T>
	static void SaveToDisk(T asset)
	{
		size_t uid = ToAssetUid(asset);
		Get().SaveToDiskInternal(Get().m_pods[uid].get());
	}

	template<CUidConvertible T>
	static void ReimportFromOriginal(T asset)
	{
		size_t uid = ToAssetUid(asset);
		Get().ReimportFromOriginalInternal(Get().m_pods[uid].get());
	}


	template<CUidConvertible T>
	static void LoadFromDiskTypeless(T asset)
	{
		size_t uid = ToAssetUid(asset);
		Get().LoadFromDiskTypelessInternal(Get().m_pods[uid].get());
	}

	template<CUidConvertible T>
	static void DeleteFromDisk(T asset)
	{
		size_t uid = ToAssetUid(asset);
		Get().DeleteFromDiskInternal(Get().m_pods[uid].get());
	}

	static void RequestGpuUpdateFor(size_t uid, AssetUpdateInfo&& info)
	{
		Get().m_gpuPodUpdateRequests.emplace_back(uid, std::move(info));
	}

	static auto& GetGpuUpdateRequests() { return Get().m_gpuPodUpdateRequests; }
	static void ClearGpuUpdateRequests() { Get().m_gpuPodUpdateRequests.clear(); }


	// Returns an alternative "valid" path for this asset. ie one that will not collide with a current asset.
	// If the passed in path is corret it will be returned instead
	static uri::Uri SuggestPath(const uri::Uri& desiredFullPath) { return Get().SuggestPathImpl(desiredFullPath); }

	template<CAssetPod PodType>
	static PodType* Z_Handle_AccessPod(size_t uid)
	{
		return static_cast<PodType*>(Get().m_pods[uid]->ptr.get());
	}

	// AVOID THIS. This is for internal use.
	[[nodiscard]] static std::vector<UniquePtr<PodEntry>>& Z_GetPods() { return Get().m_pods; }


	// Note that uri for desired path must include "gen-data/" for the pods to be loaded at runtime.
	template<CAssetPod PodType>
	static std::pair<PodEntry*, PodType*> CreateEntry(const uri::Uri& desiredPath, bool transient = false,
		const uri::Uri& originalImportLoc = "", bool reimportOnLoad = false, bool exportOnSave = false)
	{
		return Get().CreateEntryImpl<PodType>(desiredPath, transient, originalImportLoc, reimportOnLoad, exportOnSave);
	}

	// returns whether the path returned is a binary gen-data path or an actual original source file path (that can be
	// imported)
	static PathReferenceType GenerateRelativeExportPath(
		const fs::path& exporteePath, BasePodHandle dependantAsset, fs::path& outPath);

	// See export path specification on what this exports.
	// Expected to be used with AssetImporterManager::ImportOrFindFromJson
	static void GenerateRelativeExportJsonObject(
		nlohmann::json& json, const fs::path& exporteePath, BasePodHandle dependantAsset);


private:
	// This probably should reside in AssetHandlerManager.
	template<CAssetPod PodType>
	[[nodiscard]] std::pair<PodEntry*, PodType*> CreateEntryImpl(const uri::Uri& desiredPath, bool transient = false,
		const uri::Uri& originalImportLoc = "", bool reimportOnLoad = false, bool exportOnSave = false)
	{
		CLOG_ERROR(!desiredPath.starts_with("gen-data") && !transient,
			"Creating a pod entry that is not transient outside of the gen-data folder. Even after saving, this pod "
			"will not load. Are you sure this is the correct path?");

		PodEntry* e = new PodEntry();

		if (originalImportLoc.empty()) {
			reimportOnLoad = false;
			exportOnSave = false;
		}
		if (transient) {
			reimportOnLoad = false;
			exportOnSave = false;
		}

		// Populate Metadata
		e->metadata.originalImportLocation = originalImportLoc;
		e->metadata.reimportOnLoad = reimportOnLoad;
		e->metadata.exportOnSave = exportOnSave;
		e->metadata.podTypeHash = mti::GetHash<PodType>();

		// Populate entry data
		e->requiresSave = !transient;

		auto ptr = new PodType();
		e->ptr.reset(ptr);
		e->transient = transient;
		e->Z_AssignClass(&PodType::StaticClass());
		e->type = mti::GetTypeId<PodType>();


		e->path = AssetRegistry::SuggestPath(desiredPath);
		e->name = uri::GetFilename(e->path);


		e->uid = AssetRegistry::Get().m_pods.size();
		AssetRegistry::Get().m_pods.emplace_back(e);
		assetdetail::podAccessor.emplace_back(ptr);
		AssetRegistry::RegisterPathCache(e);

		return std::make_pair(e, ptr);
	}
};
