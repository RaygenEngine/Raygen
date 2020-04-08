#pragma once
#include "assets/pods/Material.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuSampler.h"

#include <vulkan/vulkan.hpp>

struct UBO_Material {
	// factors
	glm::vec4 baseColorFactor;
	glm::vec4 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;

	// alpha mask
	float alphaCutoff;
	int mask;

	float padding[2];
};

struct Material::Gpu : public vl::GpuAssetBase {

	vl::GpuHandle<Sampler> baseColorSampler;
	vl::GpuHandle<Image> baseColorImage;

	vl::GpuHandle<Sampler> metallicRoughnessSampler;
	vl::GpuHandle<Image> metallicRoughnessImage;

	vl::GpuHandle<Sampler> occlusionSampler;
	vl::GpuHandle<Image> occlusionImage;

	vl::GpuHandle<Sampler> normalSampler;
	vl::GpuHandle<Image> normalImage;

	vl::GpuHandle<Sampler> emissiveSampler;
	vl::GpuHandle<Image> emissiveImage;

	UBO_Material matData;

	UniquePtr<vl::Buffer<UBO_Material>> materialUBO;

	// one for each swapchain image
	// TODO: check
	// https://stackoverflow.com/questions/36772607/vulkan-texture-rendering-on-multiple-meshes this
	vk::DescriptorSet descriptorSet;

	Material::Gpu(PodHandle<Material> podHandle);
};
