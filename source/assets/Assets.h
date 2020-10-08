#pragma once
#include "assets/AssetManager.h"

class Assets {
	friend class Engine_;

private:
	static void Init();
	static void Destroy();

public:
	static void Import(const fs::path& path) { AssetManager->Import(path); }

	template<CAssetPod T>
	static PodHandle<T> ImportAs(const fs::path& path)
	{
		return AssetManager->ImportAs<T>(path);
	}

	template<CAssetPod T>
	static PodHandle<T> GetAsyncHandle(const uri::Uri& str)
	{
		return AssetManager->GetAsyncHandle(str);
	}

	static uri::Uri GetPodUri(BasePodHandle handle) { return AssetRegistry::GetPodUri(handle); }

}; // namespace Assets
