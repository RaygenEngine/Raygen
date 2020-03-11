#pragma once
#include "asset/pods/MaterialPod.h"
#include "renderer/asset/Texture.h"
#include "renderer/asset/GpuAssetHandle.h"


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
	int baseColorTexcoordIndex;
	int metallicRoughnessTexcoordIndex;
	int emissiveTexcoordIndex;
	int normalTexcoordIndex;
	int occlusionTexcoordIndex;

	// alpha mask
	float alphaCutoff;
	int mask;
};

template<>
struct GpuAssetBaseTyped<MaterialPod>;
using Material = GpuAssetBaseTyped<MaterialPod>;

template<>
struct GpuAssetBaseTyped<MaterialPod> : public GpuAssetBase {

	GpuHandle<TexturePod> baseColorTexture;
	GpuHandle<TexturePod> metallicRoughnessTexture;
	GpuHandle<TexturePod> occlusionTexture;
	GpuHandle<TexturePod> normalTexture;
	GpuHandle<TexturePod> emissiveTexture;

	UBO_Material matData;

	std::unique_ptr<Buffer> materialUBO;

	GpuAssetBaseTyped<MaterialPod>(PodHandle<MaterialPod> podHandle);
};
