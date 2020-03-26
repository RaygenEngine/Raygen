#pragma once
#include "assets/AssetManager.h"


class Assets {
	friend class S_Engine;

private:
	static void Init();
	static void Destroy();

public:
	static void Import(const fs::path& path) { AssetManager->Import(path); }

	template<CONC(CAssetPod) T>
	static PodHandle<T> ImportAs(const fs::path& path)
	{
		AssetManager->ImportAs<T>(path);
	}

	template<CONC(CAssetPod) T>
	static PodHandle<T> GetAsyncHandle(const uri::Uri& str)
	{
		return AssetManager->GetAsyncHandle(str);
	}

	static uri::Uri GetPodUri(BasePodHandle handle) { return AssetHandlerManager::GetPodUri(handle); }
}; // namespace Assets