#include "pch.h"
#include "rendering/asset/GpuAssetManager.h"

#include "assets/PodIncludes.h"
#include "reflection/PodTools.h"
#include "rendering/asset/Image.h"
#include "rendering/asset/Material.h"
#include "rendering/asset/Model.h"
#include "rendering/asset/Sampler.h"


#define DECLARE_LOADER(Pod)                                                                                            \
	template<>                                                                                                         \
	NOINLINE void GpuAssetManager_::Load<Pod>(PodHandle<Pod> handle)                                                   \
	{                                                                                                                  \
		gpuAssets[handle.uid].reset(new GpuAsset<Pod>(handle));                                                        \
	}

namespace vl {
DECLARE_LOADER(Model);
DECLARE_LOADER(Material);
DECLARE_LOADER(Sampler);
DECLARE_LOADER(Image);

void Dummy()
{
	GpuAssetManager->Load<Model>({});
	GpuAssetManager->Load<Material>({});
	GpuAssetManager->Load<Sampler>({});
	GpuAssetManager->Load<Image>({});
}


vk::Sampler GpuAssetManager_::GetDefaultSampler()
{
	// TODO: auto load the defaults of each asset type
	return LockHandle(GetGpuHandle<Sampler>({})).sampler.get();
}

void GpuAssetManager_::LoadAll()
{
	gpuAssets.resize(AssetHandlerManager::Z_GetPods().size());
}
} // namespace vl
