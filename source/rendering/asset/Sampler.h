#pragma once
#include "assets/pods/Sampler.h"
#include "rendering/asset/GpuAssetHandle.h"
#include "rendering/wrapper/ImageObj.h"

#include <vulkan/vulkan.hpp>

struct Sampler::Gpu : public GpuAssetBase {
	// PERF: one to many views
	vk::UniqueSampler sampler;

	Sampler::Gpu(PodHandle<Sampler> podHandle);
};
