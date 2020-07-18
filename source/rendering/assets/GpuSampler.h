#pragma once
#include "assets/pods/Sampler.h"
#include "rendering/assets/GpuAssetBase.h"

#include <vulkan/vulkan.hpp>

namespace vl {
struct GpuSampler : public GpuAssetTemplate<Sampler> {
	// PERF: one to many views
	vk::UniqueSampler sampler;

	GpuSampler(PodHandle<Sampler> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
