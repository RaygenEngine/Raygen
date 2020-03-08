#pragma once
#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"
#include "renderer/Image.h"

#include <vulkan/vulkan.hpp>

struct Texture {

	std::unique_ptr<Image> image;

	vk::UniqueImageView view;

	// PERF: one to many views
	vk::UniqueSampler sampler;

	Texture(PodHandle<TexturePod> podHandle);
};
