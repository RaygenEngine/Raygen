#pragma once

struct GpuAssetBase {
	virtual ~GpuAssetBase() = default;

	GpuAssetBase() = default;

	GpuAssetBase(const GpuAssetBase&) = delete;
	GpuAssetBase& operator=(const GpuAssetBase&) = delete;

	GpuAssetBase(GpuAssetBase&&) = default;
	GpuAssetBase& operator=(GpuAssetBase&&) = default;
};

template<CONC(CAssetPod) T>
using GpuAsset = typename T::Gpu;


template<CONC(CAssetPod) T>
struct GpuHandle : public BasePodHandle {
	GpuHandle<T>() { uid = GetDefaultPodUid<T>(); }
	GpuHandle<T>(size_t inUid) { uid = inUid; }

	[[nodiscard]] GpuAsset<T>& Lock() const { return vl::GpuAssetManager->LockHandle<T>(uid); }
};
