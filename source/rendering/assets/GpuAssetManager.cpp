#include "GpuAssetManager.h"

#include "assets/Assets.h"
#include "assets/StdAssets.h"
#include "core/iterable/IterableSafeVector.h"
#include "rendering/Device.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuSampler.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/resource/GpuResources.h"


namespace vl {
GpuAssetManager_::~GpuAssetManager_()
{
	for (auto& asset : gpuAssets) {
		delete asset;
	}
}

vk::Sampler GpuAssetManager_::GetDefaultSampler()
{
	return LockHandle(GetGpuHandle<Sampler>({})).sampler;
}

vk::Sampler GpuAssetManager_::GetShadow2dSampler()
{
	// sampler2DShadow
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(vk::Filter::eLinear) //
		.setMinFilter(vk::Filter::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
		.setAnisotropyEnable(VK_FALSE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_TRUE)
		.setCompareOp(vk::CompareOp::eLess)
		.setMipmapMode(vk::SamplerMipmapMode::eNearest)
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(32.f);

	return GpuResources::AcquireSampler(samplerInfo);
}

std::pair<GpuHandle<Image>, vk::Sampler> GpuAssetManager_::GetBrdfLutImageSampler()
{
	vk::SamplerCreateInfo samplerInfo{};
	samplerInfo
		.setMagFilter(vk::Filter::eLinear) //
		.setMinFilter(vk::Filter::eLinear)
		.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
		.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
		.setAnisotropyEnable(VK_TRUE)
		.setMaxAnisotropy(1u)
		.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
		.setUnnormalizedCoordinates(VK_FALSE)
		.setCompareEnable(VK_FALSE)
		.setCompareOp(vk::CompareOp::eAlways)
		.setMipmapMode(vk::SamplerMipmapMode::eLinear)
		.setMipLodBias(0.f)
		.setMinLod(0.f)
		.setMaxLod(32.f);

	return { GetGpuHandle(StdAssets::BrdfLut()), GpuResources::AcquireSampler(samplerInfo) };
}

void GpuAssetManager_::AllocForAll()
{
	gpuAssets.resize(AssetRegistry::Z_GetPods().size());
	gpuassetdetail::gpuAssetListData = gpuAssets.data();
}

GpuAsset<Shader>& GpuAssetManager_::CompileShader(const char* path)
{
	if (auto it = shaderPathCache.find(path); it != shaderPathCache.end()) {
		return GetGpuHandle(it->second).Lock();
	}

	PodHandle<Shader> shaderHandle = AssetRegistry::SearchForAssetFromImportPathSlow<Shader>(path);
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

	// NOTE: When impelementing this one should consider that gpu assets may have new dependecies. This requires a whole
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
	auto& updates = AssetRegistry::GetGpuUpdateRequests();
	if (!updates.empty()) {
		PerformAssetUpdates(updates);
		AssetRegistry::ClearGpuUpdateRequests();
	}
}

} // namespace vl
