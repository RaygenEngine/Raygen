#pragma once
#include "assets/pods/Sampler.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/wrappers/RImage2D.h"

#include <vulkan/vulkan.hpp>

struct Image::Gpu : public vl::GpuAssetTemplate<::Image> {
	UniquePtr<vl::RImage2D> image;

	Image::Gpu(PodHandle<Image> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
