#pragma once
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/wrappers/RCubemap.h"


struct GpuCubemap : public vl::GpuAssetTemplate<Cubemap> {
	UniquePtr<vl::RCubemap> cubemap;

	vk::DescriptorSet descriptorSet;

	GpuCubemap(PodHandle<Cubemap> podHandle);

	void Update(const AssetUpdateInfo&) override final;
};
