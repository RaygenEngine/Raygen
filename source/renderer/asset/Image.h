#pragma once
#include "asset/AssetManager.h"
#include "asset/pods/SamplerPod.h"
#include "renderer/asset/GpuAssetHandle.h"
#include "renderer/wrapper/ImageObj.h"

#include <vulkan/vulkan.hpp>

DECLARE_GPU_ASSET(Image, ImagePod)
{
	UniquePtr<ImageObj> image;
	vk::UniqueImageView view;

	GpuAssetBaseTyped<ImagePod>(PodHandle<ImagePod> podHandle);
};