#pragma once
#include "assets/pods/Material.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuSampler.h"
#include "rendering/objects/RBuffer.h"

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
	bool wip_CustomOverride{ false };

	struct wip_NewMaterialInstance {
		vk::UniquePipelineLayout plLayout; // Should actually be in Archetype (later)
		vk::UniquePipeline pipeline;

		vk::DescriptorSet descSet;

		UniquePtr<vl::RDescriptorLayout> descLayout;

		UniquePtr<vl::RBuffer> uboBuf;

		vk::UniqueShaderModule fragModule;

		vk::UniquePipelineLayout depthPlLayout;
		vk::UniquePipeline depthPipeline;

		vk::UniqueShaderModule depthFragModule;

	} wip_New;

	Material::Gpu(PodHandle<Material> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
