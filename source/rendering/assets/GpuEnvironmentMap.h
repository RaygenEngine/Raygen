#pragma once
#include "assets/pods/EnvironmentMap.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/objects/Cubemap.h"

#include <vulkan/vulkan.hpp>

// WIP: this asset is a bundle of assets and a descriptor set (it may be useless)
struct EnvironmentMap::Gpu : public vl::GpuAssetTemplate<EnvironmentMap> {
	// TODO: skybox is temp (part of skymesh)
	vl::GpuHandle<::Cubemap> skybox;
	vl::GpuHandle<::Cubemap> irradiance;
	vl::GpuHandle<::Cubemap> prefiltered;
	vl::GpuHandle<::Cubemap> brdfLut;

	vk::DescriptorSet descriptorSet;

	EnvironmentMap::Gpu(PodHandle<EnvironmentMap> podHandle);

	void Update(const AssetUpdateInfo&) override final;
};
