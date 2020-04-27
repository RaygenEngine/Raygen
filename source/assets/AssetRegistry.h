#pragma once
#include "assets/AssetPod.h"
#include "assets/PodEntry.h"
#include "assets/PodHandle.h"
#include "assets/UriLibrary.h"
#include "core/StringUtl.h"

struct Image;
struct Sampler;
struct Shader;
struct StringPod;
class ReflClass;

class AssetHandlerManager {

private:
	friend class AssetImporterManager_;
	friend class AssetManager_;

	static AssetHandlerManager& Get()
	{
		static AssetHandlerManager inst = AssetHandlerManager();
		return inst;
	}

	AssetHandlerManager() {}

	std::vector<UniquePtr<PodEntry>> m_pods;
	std::unordered_map<uri::Uri, size_t, str::HashInsensitive> m_pathCache;


	uri::Uri SuggestFilenameImpl(const fs::path& directory, const uri::Uri& desired)
	{
		uri::Uri desiredFilename = desired;

		auto dotLoc = desiredFilename.rfind('.');

		if (dotLoc != std::string::npos) {
			std::replace(desiredFilename.begin(), desiredFilename.begin() + dotLoc - 1, '.', '_');
		}

		auto desiredFullPath = (directory / desiredFilename).string();

		if (!m_pathCache.count(desiredFullPath)) {
			return desiredFilename;
		}

		// TODO: Detect and generate a correct numbered system with _1, _2 etc instead of random
		std::string resultFilename = fmt::format(
			"{}_{}{}", uri::StripExt(desiredFilename), std::rand(), uri::GetDiskExtension(desiredFilename));
		return fs::path(desiredFilename).replace_filename(resultFilename).generic_string();
	}

	uri::Uri SuggestPathImpl(const uri::Uri& desiredFullPath)
	{
		if (!m_pathCache.count(desiredFullPath)) {
			return desiredFullPath;
		}

		fs::path path = desiredFullPath;
		// TODO: Use the same system as suggest filename impl.
		path.replace_filename(path.filename().string() + fmt::format("_{}", std::rand()));
		return path.generic_string();
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

	// TODO: Check for pathCache. Currently does not affect path cache, maybe it should but why?
	// For name collision. Implement remove & write to path cache
	void RenameEntryImpl(PodEntry* entry, const std::string_view newFullPath);


	void DeleteFromDiskInternal(PodEntry* entry);

	PodEntry* DuplicateImpl(PodEntry* entry);

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
		return Get().m_pods[handle.uid]->metadata.originalImportLocation;
	}


	template<CONC(CUidConvertible) T>
	static PodEntry* Duplicate(T asset)
	{
		size_t uid = ToAssetUid(asset);
		return Get().DuplicateImpl(Get().m_pods[uid].get());
	}

	static void RemoveEntry(size_t uid) { Get().m_pods[uid].reset(); }

	static void RemoveEntry(BasePodHandle handle) { RemoveEntry(handle.uid); }

	static uri::Uri GetPodUri(BasePodHandle handle) { return Get().m_pods[handle.uid]->path; }

	static PodEntry* GetEntry(BasePodHandle handle) { return Get().m_pods[handle.uid].get(); }

	static void RenameEntry(PodEntry* entry, const std::string_view newFullPath)
	{
		Get().RenameEntryImpl(entry, newFullPath);
	}

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

	template<CONC(CUidConvertible) T>
	static void DeleteFromDisk(T asset)
	{
		size_t uid = ToAssetUid(asset);
		Get().DeleteFromDiskInternal(Get().m_pods[uid].get());
	}


	// Returns an alternative "valid" path for this asset. ie one that will not collide with a current asset.
	// If the passed in path is corret it will be returned instead
	static uri::Uri SuggestFilename(const fs::path& directory, const uri::Uri& desiredFilename)
	{
		return Get().SuggestFilenameImpl(directory, desiredFilename);
	}


	static uri::Uri SuggestPath(const uri::Uri& desiredFullPath) { return Get().SuggestPathImpl(desiredFullPath); }

	template<CONC(CAssetPod) PodType>
	static PodType* Z_Handle_AccessPod(size_t uid)
	{
		return static_cast<PodType*>(Get().m_pods[uid]->ptr.get());
	}

	// AVOID THIS. This is for internal use.
	[[nodiscard]] static std::vector<UniquePtr<PodEntry>>& Z_GetPods() { return Get().m_pods; }


	template<CONC(CAssetPod) PodType>
	static std::pair<PodEntry*, PodType*> CreateEntry(const uri::Uri& desiredPath, bool transient = false,
		const uri::Uri& originalImportLoc = "", bool reimportOnLoad = false, bool exportOnSave = false)
	{
		return Get().CreateEntryImpl<PodType>(desiredPath, transient, originalImportLoc, reimportOnLoad, exportOnSave);
	}

private:
	// This probably should reside in AssetHandlerManager.
	template<CONC(CAssetPod) PodType>
	[[nodiscard]] std::pair<PodEntry*, PodType*> CreateEntryImpl(const uri::Uri& desiredPath, bool transient = false,
		const uri::Uri& originalImportLoc = "", bool reimportOnLoad = false, bool exportOnSave = false)
	{
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


		e->path = AssetHandlerManager::SuggestPath(desiredPath);
		e->name = uri::GetFilename(e->path);


		e->uid = AssetHandlerManager::Get().m_pods.size();
		AssetHandlerManager::Get().m_pods.emplace_back(e);
		AssetHandlerManager::RegisterPathCache(e);

		return std::make_pair(e, ptr);
	}
};
