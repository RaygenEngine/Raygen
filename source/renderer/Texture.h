#pragma once


#include "asset/pods/TexturePod.h"

#include "asset/AssetManager.h"

#include <vulkan/vulkan.hpp>

struct Texture {

	vk::UniqueImage handle;
	vk::UniqueDeviceMemory memory;

	vk::UniqueImageView view;

	// PERF: one to many views
	vk::UniqueSampler sampler;

	Texture(PodHandle<TexturePod> podHandle);
};
