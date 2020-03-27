#pragma once
#include "assets/AssetManager.h"
#include "rendering/asset/GpuAssetHandle.h"

namespace vl {

inline class S_GpuAssetManager {
	std::vector<UniquePtr<GpuAssetBase>> gpuAssets;


public:
	S_GpuAssetManager() { LoadAll(); }

	template<typename T>
	void Load(PodHandle<T> handle)
	{
	}

	template<>
	void Load<ModelPod>(PodHandle<ModelPod> handle);

	template<>
	void Load<SamplerPod>(PodHandle<SamplerPod> handle);

	template<>
	void Load<MaterialPod>(PodHandle<MaterialPod> handle);

	template<>
	void Load<ImagePod>(PodHandle<ImagePod> handle);


	template<CONC(CAssetPod) T>
	GpuHandle<T> GetGpuHandle(PodHandle<T> handle)
	{
		size_t id = handle.podId;
		if (!gpuAssets[id]) {
			Load(handle);
		}
		return GpuHandle<T>{ id };
	}

	template<CONC(CAssetPod) T>
	GpuAssetBaseTyped<T>& LockHandle(GpuHandle<T> handle)
	{
		return static_cast<GpuAssetBaseTyped<T>&>(*gpuAssets[handle.uid]);
	}


	void LoadAll();
	void UnloadAll() { gpuAssets.clear(); }

} * GpuAssetManager;
} // namespace vl