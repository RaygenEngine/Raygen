#pragma once
#include "assets/pods/MaterialPod.h"
#include "rendering/asset/GpuAssetHandle.h"
#include "rendering/asset/Image.h"
#include "rendering/asset/Sampler.h"

#include <vulkan/vulkan.hpp>

struct UBO_Material {
	// factors
	glm::vec4 baseColorFactor;
	glm::vec4 emissiveFactor;
	float metallicFactor;
	float roughnessFactor;
	float normalScale;
	float occlusionStrength;

	// text coord indices
	int baseColorUvIndex;
	int metallicRoughnessUvIndex;
	int emissiveUvIndex;
	int normalUvIndex;
	int occlusionUvIndex;

	// alpha mask
	float alphaCutoff;
	int mask;
};

struct Material::Gpu : public GpuAssetBase {

	GpuHandle<Sampler> baseColorSampler;
	GpuHandle<Image> baseColorImage;

	GpuHandle<Sampler> metallicRoughnessSampler;
	GpuHandle<Image> metallicRoughnessImage;

	GpuHandle<Sampler> occlusionSampler;
	GpuHandle<Image> occlusionImage;

	GpuHandle<Sampler> normalSampler;
	GpuHandle<Image> normalImage;

	GpuHandle<Sampler> emissiveSampler;
	GpuHandle<Image> emissiveImage;

	UBO_Material matData;

	UniquePtr<Buffer> materialUBO;

	// one for each swapchain image
	// TODO: check
	// https://stackoverflow.com/questions/36772607/vulkan-texture-rendering-on-multiple-meshes this
	vk::DescriptorSet descriptorSet;

	Material::Gpu(PodHandle<Material> podHandle);
};
