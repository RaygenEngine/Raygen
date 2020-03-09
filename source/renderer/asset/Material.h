#pragma once
#include "renderer/asset/Texture.h"
#include "asset/pods/MaterialPod.h"

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


struct Material {

	std::unique_ptr<Texture> baseColorTexture;
	std::unique_ptr<Texture> metallicRoughnessTexture;
	std::unique_ptr<Texture> occlusionTexture;
	std::unique_ptr<Texture> normalTexture;
	std::unique_ptr<Texture> emissiveTexture;

	UBO_Material matData;

	std::unique_ptr<Buffer> materialUBO;

	Material(PodHandle<MaterialPod> podHandle);
};
