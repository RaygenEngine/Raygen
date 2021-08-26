#pragma once
#include "rendering/assets/GpuAssetBase.h"

#include "rendering/VkCoreIncludes.h"

namespace vl {
inline class GpuAssetManager_ {
	// NOTE: whenever any operation on this may move the vector, gpuassetdetail ::gpuAssetListData should be updated
	std::vector<GpuAssetBase*> gpuAssets;

	// We should use GpuAssetEntries if we need more than just a single array for metadata info
	//
	// Asset users stores at the specific UID the gpuAssetUsers of the asset with that uid.
	std::vector<std::vector<size_t>> assetUsers;

	std::unordered_map<std::string, PodHandle<Shader>, str::HashInsensitive> shaderPathCache;


	template<CAssetPod T>
	GpuAsset<T>& LockHandle(GpuHandle<T> handle)
	{
		return static_cast<GpuAsset<T>&>(*gpuAssets[handle.uid]);
	}

public:
	GpuAssetManager_() { AllocForAll(); }
	~GpuAssetManager_();


	template<typename T>
	void Load(PodHandle<T> handle)
	{
		delete gpuAssets[handle.uid];
		gpuAssets[handle.uid] = new GpuAsset<T>(handle);
	}

	template<CAssetPod T>
	GpuHandle<T> GetGpuHandle(PodHandle<T> handle)
	{
		size_t id = handle.uid;
		if (id >= gpuAssets.size()) [[unlikely]] {
			AllocForAll();
		}

		if (!gpuAssets[id]) [[unlikely]] {
			Load(handle);
		}

		return GpuHandle<T>{ id };
	}

	// CHECK: Std gpu assets
	vk::Sampler GetDefaultSampler();
	vk::Sampler GetShadow2dSampler();
	std::pair<GpuHandle<Image>, vk::Sampler> GetBrdfLutImageSampler();

	void AllocForAll();
	void UnloadAll()
	{
		for (auto ptr : gpuAssets) {
			delete ptr;
		}
		gpuAssets.clear();
		gpuassetdetail::gpuAssetListData = nullptr;
	}


	// Now will also cache hit so its safe to call on a non static context (ie every recompile)
	GpuAsset<Shader>& CompileShader(const char* path);


	std::vector<size_t> GetUsersFor(size_t uid);

	// The return value invalidates after calling this again with a higher uid.
	// The return value is editable (mutable).
	std::vector<size_t>& GetUsersRef(size_t uid);

	[[nodiscard]] size_t Z_GetSize() const { return gpuAssets.size(); }
	[[nodiscard]] std::vector<GpuAssetBase*>& Z_GetAssets() { return gpuAssets; }

private:
	// Updates all gpu side assets that are in use from the list to their current cpu state.
	// This does device->waitIdle. For now this synchronization is enough, when doing multithreading we should handle
	// interject this in vulkan layer, synchronize there and call this just for gpu updates or even run the updates in
	// parallel.
	void PerformAssetUpdates(std::vector<std::pair<size_t /*assetUid*/, AssetUpdateInfo>>& updates);


	void SortAssetUpdates(std::vector<std::pair<size_t /*assetUid*/, AssetUpdateInfo>>& updates);

public:
	void ConsumeAssetUpdates();
} * GpuAssetManager{};
} // namespace vl
