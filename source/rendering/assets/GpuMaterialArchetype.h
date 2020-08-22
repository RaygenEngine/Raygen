#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/DescriptorSetLayout.h"


namespace vl {
struct RDescriptorSetLayout;

struct GpuMaterialArchetype : public GpuAssetTemplate<MaterialArchetype> {
	struct PassInfo {
		vk::UniquePipelineLayout pipelineLayout;
		vk::UniquePipeline pipeline;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::UniqueShaderModule> shaderModules;
	};

	RDescriptorSetLayout descLayout;

	PassInfo gbuffer;
	PassInfo gbufferAnimated;
	PassInfo depth;
	PassInfo depthAnimated;
	PassInfo unlit;

	bool isUnlit{ false };

	GpuMaterialArchetype(PodHandle<MaterialArchetype> podHandle);
	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
