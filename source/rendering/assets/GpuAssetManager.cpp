#include "pch.h"
#include "GpuAssetManager.h"

#include "assets/Assets.h"
#include "assets/AssetRegistry.h"
#include "assets/PodIncludes.h"
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

void GpuAssetManager_::PerformAssetUpdates(const std::vector<std::pair<size_t, AssetUpdateInfo>>& updates)
{
	Device->waitIdle();

	for (auto& [uid, info] : updates) {
		if (gpuAssets[uid]) {
			gpuAssets[uid]->Update(info);
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
