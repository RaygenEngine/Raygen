#include "pch.h"
#include "rendering/asset/GpuAssetManager.h"

#include "assets/Assets.h"
#include "assets/PodIncludes.h"
#include "reflection/PodTools.h"
#include "rendering/asset/Image.h"
#include "rendering/asset/Material.h"
#include "rendering/asset/Mesh.h"
#include "rendering/asset/Sampler.h"
#include "rendering/asset/Shader.h"


#define DECLARE_LOADER(Pod)                                                                                            \
	template<>                                                                                                         \
	NOINLINE void GpuAssetManager_::Load<Pod>(PodHandle<Pod> handle)                                                   \
	{                                                                                                                  \
		gpuAssets[handle.uid].reset(new GpuAsset<Pod>(handle));                                                        \
	}

namespace vl {
DECLARE_LOADER(Mesh);
DECLARE_LOADER(Material);
DECLARE_LOADER(Sampler);
DECLARE_LOADER(Image);
DECLARE_LOADER(Shader);

void Dummy()
{
	GpuAssetManager->Load<Mesh>({});
	GpuAssetManager->Load<Material>({});
	GpuAssetManager->Load<Sampler>({});
	GpuAssetManager->Load<Image>({});
	GpuAssetManager->Load<Shader>({});
}

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
} // namespace vl
