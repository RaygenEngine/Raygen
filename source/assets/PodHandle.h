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


	[[nodiscard]] const PodType* Lock() const { return AssetHandlerManager::Z_Handle_AccessPod<PodType>(uid); }
	[[nodiscard]] const PodEntry* _Debug() const { return AssetHandlerManager::GetEntry(uid); }

	friend class AssetImporterManager;
};

// This handle should be used as a temporary rarely as a class/struct member.
template<typename PodTypeT>
struct MutablePod : PodHandle<PodTypeT> {
public:
	using PodType = PodTypeT;

	static_assert(std::is_base_of_v<AssetPod, PodType>, "Pod type should be a pod");

	PodType* BeginEdit()
	{
		auto entry = AssetHandlerManager::GetEntry(uid);
		entry->MarkSave();
		return static_cast<PodType*>(entry->ptr.get());
	}
};
