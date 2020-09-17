#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/ImageView.h"


namespace vl {
struct GpuCubemap : public GpuAssetTemplate<Cubemap> {
	RCubemap cubemap{};

	vk::DescriptorSet descriptorSet;

	GpuCubemap(PodHandle<Cubemap> podHandle);

	void Update(const AssetUpdateInfo&) override final;
};
} // namespace vl
