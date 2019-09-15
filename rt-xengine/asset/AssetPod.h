#pragma once

#include "system/reflection/Reflector.h"

struct AssetPod
{

};

struct UnloadableAssetPod : AssetPod
{
	bool isLoaded{ false };
};

template<typename N>
constexpr bool is_deletable_pod_v = std::is_base_of_v<DeletableAssetPod, N>;

template<typename PodTypeT>
struct PodHandle
{
public:
	using PodType = PodTypeT;
	static_assert(std::is_base_of_v<AssetPod, PodType>, "Pod type should be a pod");
private:
	size_t podId;

public:
	PodType* operator->()
	{
		PodType* pod = AssetManager::FindPod<PodType>(podId);
		if constexpr (is_unloadable_pod_v<PodType>)
		{
			if (pod == nullptr) 
			{
				pod = AssetManager::ReloadPod<PodType>(podId);
			}
		}
		return pod;
	}

	friend class AssetManager;
};