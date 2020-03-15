#pragma once
#include "asset/pods/MaterialPod.h"
#include "renderer/asset/GpuAssetHandle.h"
#include "renderer/asset/Image.h"
#include "renderer/asset/Sampler.h"

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

template<>
struct GpuAssetBaseTyped<MaterialPod>;
using Material = GpuAssetBaseTyped<MaterialPod>;

template<>
struct GpuAssetBaseTyped<MaterialPod> : public GpuAssetBase {

	GpuHandle<SamplerPod> baseColorSampler;
	GpuHandle<ImagePod> baseColorImage;

	GpuHandle<SamplerPod> metallicRoughnessSampler;
	GpuHandle<ImagePod> metallicRoughnessImage;

	GpuHandle<SamplerPod> occlusionSampler;
	GpuHandle<ImagePod> occlusionImage;

	GpuHandle<SamplerPod> normalSampler;
	GpuHandle<ImagePod> normalImage;

	GpuHandle<SamplerPod> emissiveSampler;
	GpuHandle<ImagePod> emissiveImage;

	UBO_Material matData;

	UniquePtr<Buffer> materialUBO;

	// one for each swapchain image
	// TODO: check
	// https://stackoverflow.com/questions/36772607/vulkan-texture-rendering-on-multiple-meshes this
	vk::DescriptorSet descriptorSet;

	GpuAssetBaseTyped<MaterialPod>(PodHandle<MaterialPod> podHandle);
};
