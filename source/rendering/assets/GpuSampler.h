#pragma once
#include "assets/pods/Sampler.h"
#include "rendering/assets/GpuAssetHandle.h"

#include <vulkan/vulkan.hpp>

struct Sampler::Gpu : public vl::GpuAssetTemplate<Sampler> {
	// PERF: one to many views
	vk::UniqueSampler sampler;

	Sampler::Gpu(PodHandle<Sampler> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
