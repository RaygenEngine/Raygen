#pragma once
#include "rendering/assets/GpuAssetBase.h"

#include "rendering/VkCoreIncludes.h"

namespace vl {
struct GpuSampler : public GpuAssetTemplate<Sampler> {
	vk::Sampler sampler;

	GpuSampler(PodHandle<Sampler> podHandle);
	~GpuSampler();

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
