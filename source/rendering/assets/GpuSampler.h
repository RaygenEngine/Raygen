#pragma once
#include "rendering/assets/GpuAssetBase.h"

namespace vl {
struct GpuSampler : public GpuAssetTemplate<Sampler> {
	vk::UniqueSampler sampler;

	GpuSampler(PodHandle<Sampler> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
