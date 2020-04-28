#pragma once

#include "assets/AssetUpdateInfo.h"

namespace vl {

// Generic base for a gpu asset
struct GpuAssetBase {
	size_t podUid{ 0 };
	virtual ~GpuAssetBase() = default;

	GpuAssetBase(BasePodHandle handle)
		: podUid(handle.uid){};

	GpuAssetBase(const GpuAssetBase&) = delete;
	GpuAssetBase& operator=(const GpuAssetBase&) = delete;

	GpuAssetBase(GpuAssetBase&&) = default;
	GpuAssetBase& operator=(GpuAssetBase&&) = default;

	virtual void Update(const AssetUpdateInfo& info) {}
};

// Templated base for a gpu asset
template<CONC(CAssetPod) PodType>
struct GpuAssetTemplate : public GpuAssetBase {
	PodHandle<PodType> podHandle;

	GpuAssetTemplate(BasePodHandle handle)
		: GpuAssetBase(handle)
		, podHandle(handle)
	{
		// We should check the underlying pod type here of the handle but that requires an asset manager include
	}
};


template<CONC(CAssetPod) T>
using GpuAsset = typename T::Gpu;


template<CONC(CAssetPod) T>
struct GpuHandle : public BasePodHandle {
	GpuHandle<T>() { uid = GetDefaultPodUid<T>(); }
	GpuHandle<T>(size_t inUid) { uid = inUid; }

	[[nodiscard]] GpuAsset<T>& Lock() const { return vl::GpuAssetManager->LockHandle<T>(uid); }
};

} // namespace vl
