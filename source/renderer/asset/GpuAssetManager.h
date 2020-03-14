#pragma once
#include "asset/AssetManager.h"
#include "renderer/asset/GpuAssetHandle.h"

inline class S_GpuAssetManager {
	std::vector<UniquePtr<GpuAssetBase>> gpuAssets;


public:
	template<typename T>
	void Load(PodHandle<T> handle)
	{
	}

	template<>
	void Load<ModelPod>(PodHandle<ModelPod> handle);

	template<>
	void Load<TexturePod>(PodHandle<TexturePod> handle);

	template<>
	void Load<MaterialPod>(PodHandle<MaterialPod> handle);

	// template<>
	// void Load<ShaderPod>(PodHandle<ShaderPod> handle);

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

} GpuAssetManager;
