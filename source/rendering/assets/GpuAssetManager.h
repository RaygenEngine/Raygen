#pragma once
#include "assets/AssetManager.h"
#include "assets/pods/Shader.h"
#include "rendering/assets/GpuAssetHandle.h"

#include <vulkan/vulkan.hpp>

namespace vl {
inline class GpuAssetManager_ {
	std::vector<UniquePtr<GpuAssetBase>> gpuAssets;


public:
	GpuAssetManager_() { AllocForAll(); }

	template<typename T>
	void Load(PodHandle<T> handle)
	{
		gpuAssets[handle.uid].reset(new GpuAsset<T>(handle));
	}

	template<CONC(CAssetPod) T>
	GpuHandle<T> GetGpuHandle(PodHandle<T> handle)
	{
		size_t id = handle.uid;
		if (id >= gpuAssets.size()) {
			AllocForAll();
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

	void AllocForAll();
	void UnloadAll() { gpuAssets.clear(); }


	GpuAsset<Shader>& CompileShader(const fs::path& path);

	void ShaderChanged(PodHandle<Shader> handle);


private:
	// Updates all gpu side assets that are in use from the list to their current cpu state.
	// This does device->waitIdle. For now this synchronization is enough, when doing multithreading we should handle
	// interject this in vulkan layer, synchronize there and call this just for gpu updates or even run the updates in
	// parallel.
	void PerformAssetUpdates(const std::vector<std::pair<size_t /*assetUid*/, AssetUpdateInfo>>& updates);

public:
	void ConsumeAssetUpdates();
} * GpuAssetManager{};
} // namespace vl
