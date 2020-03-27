#pragma once
#include "assets/pods/SamplerPod.h"
#include "rendering/asset/GpuAssetHandle.h"
#include "rendering/wrapper/ImageObj.h"

#include <vulkan/vulkan.hpp>

DECLARE_GPU_ASSET(Image, ImagePod)
{
	UniquePtr<ImageObj> image;
	vk::UniqueImageView view;

	GpuAssetBaseTyped<ImagePod>(PodHandle<ImagePod> podHandle);
};
