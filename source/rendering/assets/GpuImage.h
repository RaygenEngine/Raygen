#pragma once
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/wrappers/RImage2D.h"


struct GpuImage : public vl::GpuAssetTemplate<::Image> {
	UniquePtr<vl::RImage2D> image;

	GpuImage(PodHandle<Image> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
