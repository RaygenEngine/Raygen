#pragma once
#include "assets/AssetPod.h"
#include "assets/PodFwd.h"

struct PodEntry;

template<typename PodTypeT>
struct PodHandle : BasePodHandle {
	using PodType = PodTypeT;

private:
	constexpr static size_t s_defaultUid = GetDefaultPodUid<PodType>();

public:
	constexpr PodHandle() { uid = s_defaultUid; }
	constexpr PodHandle(size_t id) { uid = id; }
	constexpr PodHandle(BasePodHandle handle) { uid = handle.uid; }
	[[nodiscard]] constexpr bool IsDefault() const { return uid == s_defaultUid; }


	//[[nodiscard]] const PodEntry* _Debug() const { return AssetHandlerManager::GetEntry(*this); }
	[[nodiscard]] const PodType* Lock() const { return static_cast<PodType*>(assetdetail::podAccessor[uid]); }

	friend class AssetImporterManager_;
};
