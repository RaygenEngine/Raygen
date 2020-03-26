#pragma once
#include "assets/AssetPod.h"
#include "assets/PodHandle.h"
#include "assets/PodEntry.h"
#include "core/StringUtl.h"
#include "assets/UriLibrary.h"


struct ImagePod;
struct SamplerPod;
struct ShaderPod;
struct StringPod;
class ReflClass;


class AssetHandlerManager {

private:
	friend class S_AssetImporterManager;
	friend class S_AssetManager;

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
