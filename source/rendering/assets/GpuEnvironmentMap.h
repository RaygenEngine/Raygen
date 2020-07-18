#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/RCubemap.h"

namespace vl {
struct GpuEnvironmentMap : public GpuAssetTemplate<EnvironmentMap> {

	GpuHandle<Cubemap> skybox;
	GpuHandle<Cubemap> irradiance;
	GpuHandle<Cubemap> prefiltered;
	GpuHandle<Image> brdfLut;

	vk::DescriptorSet descriptorSet;

	vk::UniqueSampler brdfSampler;

	GpuEnvironmentMap(PodHandle<EnvironmentMap> podHandle);

	void Update(const AssetUpdateInfo&) override final;
};

} // namespace vl
