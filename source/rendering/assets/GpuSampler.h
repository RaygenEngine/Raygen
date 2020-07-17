#pragma once
#include "assets/pods/Sampler.h"
#include "rendering/assets/GpuAssetHandle.h"

#include <vulkan/vulkan.hpp>

struct GpuSampler : public vl::GpuAssetTemplate<Sampler> {
	// PERF: one to many views
	vk::UniqueSampler sampler;

	GpuSampler(PodHandle<Sampler> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
