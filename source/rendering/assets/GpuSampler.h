#pragma once
#include "assets/pods/Sampler.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/objects/Image.h"

#include <vulkan/vulkan.hpp>

struct Sampler::Gpu : public vl::GpuAssetBase {
	// PERF: one to many views
	vk::UniqueSampler sampler;

	Sampler::Gpu(PodHandle<Sampler> podHandle);
};
