#pragma once
#include "assets/AssetPod.h"
#include "assets/PodFwd.h"

struct PodEntry;

template<typename PodTypeT>
struct PodHandle : BasePodHandle {
public:
	using PodType = PodTypeT;

	constexpr PodHandle() { uid = GetDefaultPodUid<PodType>(); }
	constexpr PodHandle(size_t id) { uid = id; }
	constexpr PodHandle(BasePodHandle handle) { uid = handle.uid; }


	static_assert(std::is_base_of_v<AssetPod, PodType>, "Pod type should be a pod");

	[[nodiscard]] const PodEntry* _Debug() const { return AssetHandlerManager::GetEntry(*this); }


	[[nodiscard]] const PodType* Lock() const { return AssetHandlerManager::Z_Handle_AccessPod<PodType>(uid); }

	friend class AssetImporterManager;
};
