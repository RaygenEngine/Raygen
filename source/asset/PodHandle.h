#pragma once

#include "asset/AssetPod.h"
#include "system/Engine.h"

struct PodEntry;

template<typename PodTypeT>
struct PodHandle : BasePodHandle {
public:
	using PodType = PodTypeT;
	static_assert(std::is_base_of_v<AssetPod, PodType>, "Pod type should be a pod");

	[[nodiscard]] const PodEntry* _Debug() const { return Engine::GetAssetManager()->GetEntry(*this); }


	[[nodiscard]] const PodType* Lock() const
	{
		return Engine::GetAssetManager()->_Handle_AccessPod<PodType>(podId);
	}

	friend class AssetManager;
};
