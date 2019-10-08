#pragma once
#include "asset/AssetPod.h"

struct PodEntry;

template<typename PodTypeT>
struct PodHandle : BasePodHandle {
public:
	using PodType = PodTypeT;
	static_assert(std::is_base_of_v<AssetPod, PodType>, "Pod type should be a pod");

	const PodEntry* _Debug() const { return Engine::GetAssetManager()->GetEntry(*this); }

	const PodType* operator->() const { return Engine::GetAssetManager()->_Handle_AccessPod<PodType>(podId); }

	//
	//
	//
	//
	const PodType* Lock() const { return operator->(); }

	friend class AssetManager;
};