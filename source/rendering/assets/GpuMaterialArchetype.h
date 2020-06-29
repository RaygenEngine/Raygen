#pragma once
#include "assets/pods/MaterialArchetype.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuSampler.h"
#include "rendering/wrappers/RBuffer.h"

#include <vulkan/vulkan.hpp>

struct MaterialArchetype::Gpu : public vl::GpuAssetTemplate<MaterialArchetype> {
	struct PassInfo {
		vk::UniquePipelineLayout pipelineLayout;
		vk::UniquePipeline pipeline;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::UniqueShaderModule> shaderModules;
	};

	UniquePtr<vl::RDescriptorLayout> descLayout;

	PassInfo gbuffer;
	PassInfo depth;

	MaterialArchetype::Gpu(PodHandle<MaterialArchetype> podHandle);
	void Update(const AssetUpdateInfo& info) override final;
};
