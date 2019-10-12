#pragma once

//
// Generic Gpu Asset Manager
//
#include "asset/PodHandle.h"

template<typename GpuAssetBaseType>
class GenericGpuAssetManager {
	std::unordered_map<size_t, std::unique_ptr<GpuAssetBaseType>> m_assetMap;

public:
	template<typename AssetT>
	AssetT* GpuGetOrCreate(PodHandle<typename AssetT::PodType> handle)
	{
		size_t uid = handle.podId;
		auto it = m_assetMap.find(uid);
		if (it != m_assetMap.end()) {
			return dynamic_cast<AssetT*>(it->second.get());
		}

		AssetT* result = new AssetT(handle);
		m_assetMap.emplace(uid, result);
		result->FriendLoad();
		return result;
	}

	template<typename AssetT>
	AssetT* GenerateFromPodPath(const uri::Uri& path)
	{
		return GpuGetOrCreate<AssetT>(AssetManager::GetOrCreate<typename AssetT::PodType>(path));
	}
};
