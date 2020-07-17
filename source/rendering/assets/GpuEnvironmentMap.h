#pragma once
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/wrappers/RCubemap.h"

#include <vulkan/vulkan.hpp>

// WIP: this asset is a bundle of assets and a descriptor set (it may be useless)
struct GpuEnvironmentMap : public vl::GpuAssetTemplate<EnvironmentMap> {
	// TODO: skybox is temp (part of skymesh)
	vl::GpuHandle<Cubemap> skybox;
	vl::GpuHandle<Cubemap> irradiance;
	vl::GpuHandle<Cubemap> prefiltered;
	// TODO: this should only be loaded once and shared across all environment maps
	vl::GpuHandle<Image> brdfLut;

	vk::DescriptorSet descriptorSet;

	vk::UniqueSampler brdfSampler;

	GpuEnvironmentMap(PodHandle<EnvironmentMap> podHandle);

	void Update(const AssetUpdateInfo&) override final;
};
