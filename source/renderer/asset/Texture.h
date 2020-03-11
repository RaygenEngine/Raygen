#pragma once
#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"
#include "renderer/asset/GpuAssetHandle.h"
#include "renderer/wrapper/Image.h"

#include <vulkan/vulkan.hpp>

DECLARE_GPU_ASSET(Texture, TexturePod)
{
	std::unique_ptr<Image> image;
	vk::UniqueImageView view;

	// PERF: one to many views
	vk::UniqueSampler sampler;


	GpuAssetBaseTyped<TexturePod>(PodHandle<TexturePod> podHandle);
};
