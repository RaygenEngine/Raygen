#include "pch.h"
#include "GpuAssetManager.h"

#include "assets/Assets.h"
#include "assets/AssetRegistry.h"
#include "assets/PodIncludes.h"
#include "core/iterable/IterableSafeVector.h"
#include "reflection/PodTools.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/assets/GpuMaterialArchetype.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuSampler.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/Device.h"
#include "assets/pods/Mesh.h"

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

GpuAsset<Shader>& GpuAssetManager_::CompileShader(const char* path)
{
	if (auto it = shaderPathCache.find(path); it != shaderPathCache.end()) {
		return GetGpuHandle(it->second).Lock();
	}

	PodHandle<Shader> shaderHandle = AssetHandlerManager::SearchForAssetFromImportPathSlow<Shader>(path);
	if (shaderHandle.IsDefault()) {
		AssetImporterManager->PushPath("shaders/");
		shaderHandle = Assets::ImportAs<Shader>(fs::path(path));
		AssetImporterManager->PopPath();
	}
	shaderPathCache.emplace(path, shaderHandle);
	return GetGpuHandle(shaderHandle).Lock();
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


void GpuAssetManager_::SortAssetUpdates(std::vector<std::pair<size_t, AssetUpdateInfo>>& updates)
{
	// Sort asset updates based on dependencies to avoid double updates & ensure proper updating order

	// NOTE: When impelementing this one should consider that gpu assets may have new dependecies. This requres a whole
	// system that would allow early exits in the update function of gpu assets. ie evaluate new dependicies, check if
	// they are 'dirty' and update later (when they will not be dirty)
	// TODO:
}

void GpuAssetManager_::PerformAssetUpdates(std::vector<std::pair<size_t, AssetUpdateInfo>>& updates)
{
	SortAssetUpdates(updates);
	Device->waitIdle();

	// PERF: Unordered Set to avoid multiple updates of the same asset and remove the ugly erase unique below
	IterableSafeVector<size_t> dependentUpdates;


	decltype(updates.begin()) currentIt;

	for (currentIt = updates.begin(); currentIt != updates.end(); currentIt++) {
		auto& [uid, info] = *currentIt;

		if (uid < gpuAssets.size() && gpuAssets[uid]) {

			auto shouldUpdateNow = [&]() {
				auto& deps = gpuAssets[uid]->GetDependencies();

				// For each "parent" asset, check if it is in the remaining updates.
				for (size_t parentUid : deps) {
					auto it = std::find_if(
						currentIt + 1, updates.end(), [&](auto& pair) { return parentUid == pair.first; });
					if (it != updates.end()) {
						return false;
					}
				}
				return true;
			};


			if (shouldUpdateNow()) {
				gpuAssets[uid]->Update(info);
				if (uid < assetUsers.size()) {
					auto& usersVec = assetUsers[uid];
					dependentUpdates.vec.insert(dependentUpdates.vec.end(), usersVec.begin(), usersVec.end());
				}
			}
			// No need to add anything, it will get added to dependentUpdates when the update for the "parent" will
			// occur
		}
	}


	// PERF: can be done without multiple emplaces
	// NOTE: If you get an infinite loop here, there is a circular dependency in gpu assets.
	while (dependentUpdates.ConsumingRegion()) {
		auto& vecRef = dependentUpdates.vec;
		vecRef.erase(std::unique(vecRef.begin(), vecRef.end()), vecRef.end());

		for (auto& elem : vecRef) {
			if (elem < gpuAssets.size() && gpuAssets[elem]) {
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
