#include "pch.h"

#include "renderer/asset/GpuAssetManager.h"
#include "asset/PodIncludes.h"
#include "reflection/PodTools.h"

#include "renderer/asset/Model.h"
#include "renderer/asset/Material.h"
#include "renderer/asset/Texture.h"


#define DECLARE_LOADER(Struct, Pod)                                                                                    \
	template<>                                                                                                         \
	NOINLINE void S_GpuAssetManager::Load<Pod>(PodHandle<Pod> handle)                                                  \
	{                                                                                                                  \
		gpuAssets[handle.podId] = std::make_unique<Struct>(handle);                                                    \
	}

DECLARE_LOADER(Model, ModelPod);
DECLARE_LOADER(Material, MaterialPod);
DECLARE_LOADER(Texture, TexturePod);

void Dummy()
{
	GpuAssetManager.Load<ModelPod>({});
	GpuAssetManager.Load<MaterialPod>({});
	GpuAssetManager.Load<TexturePod>({});
}


void S_GpuAssetManager::LoadAll()
{
	gpuAssets.resize(AssetHandlerManager::Z_GetPods().size());
}
