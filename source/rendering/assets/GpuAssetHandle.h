#pragma once
// Included in pch

struct GpuAssetBase;

namespace gpuassetdetail {
// Updated from Gpu Asset Manager
inline GpuAssetBase** gpuAssetListData{ nullptr };
} // namespace gpuassetdetail

template<CAssetPod T>
struct GpuHandle : public BasePodHandle {
	GpuHandle<T>() { uid = GetDefaultPodUid<T>(); }
	GpuHandle<T>(size_t inUid) { uid = inUid; }

	[[nodiscard]] GpuAsset<T>& Lock() const
	{
		return static_cast<GpuAsset<T>&>(*gpuassetdetail::gpuAssetListData[uid]);
	}
};
