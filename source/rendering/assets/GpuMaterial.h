#pragma once
#include "assets/pods/Material.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuSampler.h"
#include "rendering/wrappers/RBuffer.h"

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
};

struct Material::Gpu : public vl::GpuAssetTemplate<Material> {
	vk::UniquePipelineLayout plLayout; // Should actually be in Archetype (later)
	vk::UniquePipeline pipeline;

	// bool hasDescriptor{ false };
	UniquePtr<vl::RDescriptorLayout> descLayout;
	vk::DescriptorSet descSet;

	UniquePtr<vl::RBuffer> uboBuf;

	vk::UniqueShaderModule fragModule;

	vk::UniquePipelineLayout depthPlLayout;
	vk::UniquePipeline depthPipeline;

	vk::UniqueShaderModule depthFragModule;

	bool hasDescriptorSet{ false };
	Material::Gpu(PodHandle<Material> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
