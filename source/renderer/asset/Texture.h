#pragma once
#include "asset/AssetManager.h"
#include "asset/pods/TexturePod.h"
#include "renderer/asset/GpuAssetHandle.h"
#include "renderer/wrapper/Image.h"

#include <vulkan/vulkan.hpp>

DECLARE_GPU_ASSET(Texture, TexturePod)
{
	UniquePtr<Image> image;
	vk::UniqueImageView view;

	// PERF: one to many views
	vk::UniqueSampler sampler;

	GpuAssetBaseTyped<TexturePod>(PodHandle<TexturePod> podHandle);

	inline static std::optional<vk::DescriptorSet> editorDescSet; // NEXT:
	inline static PodHandle<TexturePod> lastEditorPod;

	vk::DescriptorSet GetDebugDescriptor();
};
