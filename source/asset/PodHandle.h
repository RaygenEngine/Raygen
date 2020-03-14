#pragma once

#include "asset/AssetPod.h"
#include "asset/PodFwd.h"
#include "engine/Engine.h"

struct PodEntry;


template<typename PodTypeT>
struct PodHandle : BasePodHandle {
public:
	using PodType = PodTypeT;

	constexpr PodHandle() { podId = GetDefaultPodUid<PodType>(); }
	constexpr PodHandle(size_t id) { podId = id; }
	constexpr PodHandle(BasePodHandle handle) { podId = handle.podId; }


	static_assert(std::is_base_of_v<AssetPod, PodType>, "Pod type should be a pod");

	[[nodiscard]] const PodEntry* _Debug() const { return AssetHandlerManager::GetEntry(*this); }


	[[nodiscard]] const PodType* Lock() const { return AssetHandlerManager::Z_Handle_AccessPod<PodType>(podId); }

	[[nodiscard]] PodType* Z_GetMutable() { return AssetHandlerManager::Z_Handle_AccessPod<PodType>(podId); }


	friend class AssetImporterManager;
};
