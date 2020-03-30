#pragma once
#include "assets/AssetManager.h"
#include "rendering/asset/GpuAssetHandle.h"

#include <vulkan/vulkan.hpp>

namespace vl {

inline class GpuAssetManager_ {
	std::vector<UniquePtr<GpuAssetBase>> gpuAssets;


public:
	GpuAssetManager_() { LoadAll(); }

	template<typename T>
	void Load(PodHandle<T> handle)
	{
	}

	template<>
	void Load<Model>(PodHandle<Model> handle);

	template<>
	void Load<Sampler>(PodHandle<Sampler> handle);

	template<>
	void Load<Material>(PodHandle<Material> handle);

	template<>
	void Load<Image>(PodHandle<Image> handle);

	template<>
	void Load<Shader>(PodHandle<Shader> handle);


	template<CONC(CAssetPod) T>
	GpuHandle<T> GetGpuHandle(PodHandle<T> handle)
	{
		size_t id = handle.uid;
		if (id >= gpuAssets.size()) {
			gpuAssets.resize(id + 1);
		}
		if (!gpuAssets[id]) {
			Load(handle);
		}
		return GpuHandle<T>{ id };
	}

	template<CONC(CAssetPod) T>
	GpuAsset<T>& LockHandle(GpuHandle<T> handle)
	{
		return static_cast<GpuAsset<T>&>(*gpuAssets[handle.uid]);
	}


	vk::Sampler GetDefaultSampler();

	void LoadAll();
	void UnloadAll() { gpuAssets.clear(); }

} * GpuAssetManager{};
} // namespace vl
