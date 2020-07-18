#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/RCubemap.h"


namespace vl {
struct GpuCubemap : public GpuAssetTemplate<Cubemap> {
	UniquePtr<vl::RCubemap> cubemap;

	vk::DescriptorSet descriptorSet;

	GpuCubemap(PodHandle<Cubemap> podHandle);

	void Update(const AssetUpdateInfo&) override final;
};
} // namespace vl
