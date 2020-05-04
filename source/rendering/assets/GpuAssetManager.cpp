#include "pch.h"
#include "GpuAssetManager.h"

#include "assets/Assets.h"
#include "assets/AssetRegistry.h"
#include "assets/PodIncludes.h"
#include "core/iterable/IterableSafeVector.h"
#include "reflection/PodTools.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuMaterial.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuSampler.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/Device.h"

namespace vl {
vk::Sampler GpuAssetManager_::GetDefaultSampler()
{
	// TODO: auto load the defaults of each asset type
	return LockHandle(GetGpuHandle<Sampler>({})).sampler.get();
}

void GpuAssetManager_::AllocForAll()
{
	gpuAssets.resize(AssetHandlerManager::Z_GetPods().size());
}
GpuAsset<Shader>& GpuAssetManager_::CompileShader(const fs::path& path)
{
	auto shaderHandle = Assets::ImportAs<Shader>(path);
	return GetGpuHandle(shaderHandle).Lock();
}

void GpuAssetManager_::ShaderChanged(PodHandle<Shader> handle)
{
	GetGpuHandle(handle).Lock().Z_Recompile();
}

std::vector<size_t> GpuAssetManager_::GetUsersFor(size_t uid)
{
	if (uid >= assetUsers.size()) {
		return {};
	}
	return assetUsers[uid];
}

std::vector<size_t>& GpuAssetManager_::GetUsersRef(size_t uid)
{
	if (uid >= assetUsers.size()) {
		assetUsers.resize(uid + 1);
	}
	return assetUsers[uid];
}

void GpuAssetManager_::PerformAssetUpdates(const std::vector<std::pair<size_t, AssetUpdateInfo>>& updates)
{
	Device->waitIdle();

	// PERF: Unordered Set to avoid multiple updates of the same asset and remove the ugly erase unique below
	IterableSafeVector<size_t> dependentUpdates;


	for (auto& [uid, info] : updates) {
		if (gpuAssets[uid]) {
			gpuAssets[uid]->Update(info);
		}
		if (uid < assetUsers.size()) {
			auto& usersVec = assetUsers[uid];
			dependentUpdates.vec.insert(dependentUpdates.vec.begin(), usersVec.begin(), usersVec.end());
		}
	}


	// PERF: can be done without multiple emplaces
	// NOTE: If you get an infinite loop here, there is a circular dependency in gpu assets.
	while (dependentUpdates.ConsumingRegion()) {
		auto& vecRef = dependentUpdates.vec;
		vecRef.erase(std::unique(vecRef.begin(), vecRef.end()), vecRef.end());

		for (auto& elem : vecRef) {
			if (gpuAssets[elem]) {
				gpuAssets[elem]->Update({}); // TODO: add specific update
			}
			if (elem < assetUsers.size()) {
				for (auto& user : assetUsers[elem]) {
					dependentUpdates.Emplace(user);
				}
			}
		}
	}
}

void GpuAssetManager_::ConsumeAssetUpdates()
{
	auto& updates = AssetHandlerManager::GetGpuUpdateRequests();
	if (!updates.empty()) {
		PerformAssetUpdates(updates);
		AssetHandlerManager::ClearGpuUpdateRequests();
	}
}

} // namespace vl
