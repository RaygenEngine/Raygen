#pragma once

#include "ctti/type_id.hpp"

#include <filesystem>

namespace fs = std::filesystem;

struct AssetPod
{
	ctti::type_id_t type;
};

struct BasePodHandle
{
protected:
	size_t podId{ 0 };
};

template<typename PodTypeT>
struct PodHandle : BasePodHandle
{
public:
	using PodType = PodTypeT;
	static_assert(std::is_base_of_v<AssetPod, PodType>, "Pod type should be a pod");


	PodType* DebugGetPointer() const 
	{
		return Engine::GetAssetManager()->_Internal_MaybeFindPod<PodType>(podId);
	}

public:
	PodType* operator->() const
	{
		assert(podId != 0);
		PodType* pod = Engine::GetAssetManager()->_Internal_MaybeFindPod<PodType>(podId);
		// TODO: If deletable pods get implemented, load on create non deletable pods in asset manager.
		//if constexpr (is_deletable_pod_v<PodType>)
		//{
			if (pod == nullptr) 
			{
				pod = Engine::GetAssetManager()->_Internal_RefreshPod<PodType>(podId);
			}
		//}
		assert(pod);
		return pod;
	}

	bool HasBeenAssigned() const
	{
		return podId != 0;
	}

	bool operator==(const PodHandle<PodType>& other) const
	{
		return other.podId == this->podId && podId != 0;
	}

	friend class AssetManager;
	friend struct std::hash<PodHandle<PodTypeT>>;
};

namespace std {
template<typename PodT> 
struct hash<PodHandle<PodT>>
{
	size_t operator()(const PodHandle<PodT>& x) const
	{
		return x.podId;
	}
};
}
