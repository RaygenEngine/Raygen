#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/Image.h"

namespace vl {
struct GpuEnvironmentMap : public GpuAssetTemplate<EnvironmentMap> {

	GpuHandle<Cubemap> skybox;
	GpuHandle<Cubemap> irradiance;
	GpuHandle<Cubemap> prefiltered;
	GpuHandle<Image> brdfLut;

	vk::DescriptorSet descriptorSet;

	vk::Sampler brdfSampler;

	GpuEnvironmentMap(PodHandle<EnvironmentMap> podHandle);
	~GpuEnvironmentMap();

	void Update(const AssetUpdateInfo&) override final;
};

} // namespace vl
