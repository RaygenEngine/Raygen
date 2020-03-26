#include "pch.h"
#include "renderer/asset/GpuAssetManager.h"

#include "assets/PodIncludes.h"
#include "reflection/PodTools.h"
#include "renderer/asset/Image.h"
#include "renderer/asset/Material.h"
#include "renderer/asset/Model.h"
#include "renderer/asset/Sampler.h"


#define DECLARE_LOADER(Struct, Pod)                                                                                    \
	template<>                                                                                                         \
	NOINLINE void S_GpuAssetManager::Load<Pod>(PodHandle<Pod> handle)                                                  \
	{                                                                                                                  \
		gpuAssets[handle.podId].reset(new Struct(handle));                                                             \
	}

DECLARE_LOADER(Model, ModelPod);
DECLARE_LOADER(Material, MaterialPod);
DECLARE_LOADER(Sampler, SamplerPod);
DECLARE_LOADER(Image, ImagePod);

void Dummy()
{
	GpuAssetManager.Load<ModelPod>({});
	GpuAssetManager.Load<MaterialPod>({});
	GpuAssetManager.Load<SamplerPod>({});
	GpuAssetManager.Load<ImagePod>({});
}


void S_GpuAssetManager::LoadAll()
{
	gpuAssets.resize(AssetHandlerManager::Z_GetPods().size());
}
