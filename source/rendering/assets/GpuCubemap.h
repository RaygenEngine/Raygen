#pragma once
#include "assets/pods/Cubemap.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/wrappers/RCubemap.h"

#include <vulkan/vulkan.hpp>

struct GpuCubemap : public vl::GpuAssetTemplate<Cubemap> {
	UniquePtr<vl::RCubemap> cubemap;

	vk::DescriptorSet descriptorSet;

	GpuCubemap(PodHandle<Cubemap> podHandle);

	void Update(const AssetUpdateInfo&) override final;
};
