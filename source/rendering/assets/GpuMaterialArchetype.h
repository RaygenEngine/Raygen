#pragma once
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuSampler.h"

#include <vulkan/vulkan.hpp>

namespace vl {
struct GpuMaterialArchetype : public vl::GpuAssetTemplate<MaterialArchetype> {
	struct PassInfo {
		vk::UniquePipelineLayout pipelineLayout;
		vk::UniquePipeline pipeline;
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::UniqueShaderModule> shaderModules;
	};

	UniquePtr<vl::RDescriptorLayout> descLayout;

	PassInfo gbuffer;
	PassInfo gbufferAnimated;
	PassInfo depth;
	PassInfo depthAnimated;

	GpuMaterialArchetype(PodHandle<MaterialArchetype> podHandle);
	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
