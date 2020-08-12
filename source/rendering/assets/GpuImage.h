#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/RImage.h"


namespace vl {
struct GpuImage : public GpuAssetTemplate<::Image> {
	RImage2D image;

	GpuImage(PodHandle<Image> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
