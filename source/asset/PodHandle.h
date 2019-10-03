#pragma once
#include "asset/AssetPod.h"

template<typename PodTypeT>
struct PodHandle : BasePodHandle
{

public:
	using PodType = PodTypeT;
	static_assert(std::is_base_of_v<AssetPod, PodType>, "Pod type should be a pod");

	//const PodEntry& _Debug() const
	//{
	//	return Engine::GetAssetManager()->m_pods[podId];
	//}

	PodType* operator->() const
	{
		return Engine::GetAssetManager()->_Handle_AccessPod<PodType>(podId);
	}

	friend class AssetManager;
};