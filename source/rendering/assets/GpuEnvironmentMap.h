#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/ImageView.h"

namespace vl {
struct GpuEnvironmentMap : public GpuAssetTemplate<EnvironmentMap> {

	GpuHandle<Cubemap> skybox;
	GpuHandle<Cubemap> irradiance;
	GpuHandle<Cubemap> prefiltered;

	vk::DescriptorSet descriptorSet;

	GpuEnvironmentMap(PodHandle<EnvironmentMap> podHandle);

	void Update(const AssetUpdateInfo&) override final;
};

} // namespace vl
