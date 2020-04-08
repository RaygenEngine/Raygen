#pragma once
#include "assets/pods/Sampler.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/objects/Image2D.h"

#include <vulkan/vulkan.hpp>

struct ::Image::Gpu : public vl::GpuAssetBase {
	UniquePtr<vl::Image2D> image;

	Image::Gpu(PodHandle<Image> podHandle);
};
