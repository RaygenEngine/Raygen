#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/ImageView.h"

namespace vl {
struct GpuCubemap : public GpuAssetTemplate<Cubemap> {
	RCubemap cubemap{};

	GpuCubemap(PodHandle<Cubemap> podHandle);

	void Update(const AssetUpdateInfo&) override final;
};
} // namespace vl
