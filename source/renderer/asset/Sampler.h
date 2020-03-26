#pragma once
#include "assets/AssetManager.h"
#include "assets/pods/SamplerPod.h"
#include "renderer/asset/GpuAssetHandle.h"
#include "renderer/wrapper/ImageObj.h"

#include <vulkan/vulkan.hpp>

DECLARE_GPU_ASSET(Sampler, SamplerPod)
{
	// PERF: one to many views
	vk::UniqueSampler sampler;

	GpuAssetBaseTyped<SamplerPod>(PodHandle<SamplerPod> podHandle);
};
