#pragma once
#include "assets/pods/SamplerPod.h"
#include "rendering/asset/GpuAssetHandle.h"
#include "rendering/wrapper/ImageObj.h"

#include <vulkan/vulkan.hpp>

struct Image::Gpu : public GpuAssetBase {
	UniquePtr<ImageObj> image;
	vk::UniqueImageView view;

	Image::Gpu(PodHandle<Image> podHandle);
};
