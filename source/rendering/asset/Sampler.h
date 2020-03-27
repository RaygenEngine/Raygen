#pragma once
#include "assets/pods/SamplerPod.h"
#include "rendering/asset/GpuAssetHandle.h"
#include "rendering/wrapper/ImageObj.h"

#include <vulkan/vulkan.hpp>

DECLARE_GPU_ASSET(Sampler, SamplerPod)
{
	// PERF: one to many views
	vk::UniqueSampler sampler;

	GpuAssetBaseTyped<SamplerPod>(PodHandle<SamplerPod> podHandle);
};
