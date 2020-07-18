#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/RImage2D.h"


namespace vl {
struct GpuImage : public GpuAssetTemplate<::Image> {
	UniquePtr<RImage2D> image;

	GpuImage(PodHandle<Image> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
