#pragma once

struct GpuAssetBase {
	size_t uid{ 0 };
	virtual ~GpuAssetBase() = default;

	GpuAssetBase() = default;

	GpuAssetBase(const GpuAssetBase&) = delete;
	GpuAssetBase& operator=(const GpuAssetBase&) = delete;

	GpuAssetBase(GpuAssetBase&&) = default;
	GpuAssetBase& operator=(GpuAssetBase&&) = default;
};

template<CONC(CAssetPod) T>
struct GpuAssetBaseTyped : public GpuAssetBase {
};


struct GpuHandleBase {
	size_t uid{ 0 };
};

template<CONC(CAssetPod) T>
struct GpuHandle : public GpuHandleBase {
	// GpuAssetBaseTyped<T>& Lock() { GpuAssetManager->Get<T>(uid); }
	GpuHandle<T>() { uid = GetDefaultPodUid<T>(); }
	GpuHandle<T>(size_t inUid) { uid = inUid; }
};


#define DECLARE_GPU_ASSET(StructName, PodType)                                                                         \
	template<>                                                                                                         \
	struct GpuAssetBaseTyped<PodType>;                                                                                 \
	using StructName = GpuAssetBaseTyped<PodType>;                                                                     \
                                                                                                                       \
	template<>                                                                                                         \
	struct GpuAssetBaseTyped<PodType> : public GpuAssetBase