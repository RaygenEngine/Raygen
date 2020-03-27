#include "pch.h"
#include "rendering/asset/GpuAssetManager.h"

#include "assets/PodIncludes.h"
#include "reflection/PodTools.h"
#include "rendering/asset/Image.h"
#include "rendering/asset/Material.h"
#include "rendering/asset/Model.h"
#include "rendering/asset/Sampler.h"


#define DECLARE_LOADER(Struct, Pod)                                                                                    \
	template<>                                                                                                         \
	NOINLINE void S_GpuAssetManager::Load<Pod>(PodHandle<Pod> handle)                                                  \
	{                                                                                                                  \
		gpuAssets[handle.podId].reset(new Struct(handle));                                                             \
	}

namespace vl {
DECLARE_LOADER(Model, ModelPod);
DECLARE_LOADER(Material, MaterialPod);
DECLARE_LOADER(Sampler, SamplerPod);
DECLARE_LOADER(Image, ImagePod);

void Dummy()
{
	GpuAssetManager->Load<ModelPod>({});
	GpuAssetManager->Load<MaterialPod>({});
	GpuAssetManager->Load<SamplerPod>({});
	GpuAssetManager->Load<ImagePod>({});
}


void S_GpuAssetManager::LoadAll()
{
	gpuAssets.resize(AssetHandlerManager::Z_GetPods().size());
}
} // namespace vl
