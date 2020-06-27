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
		std::vector<vk::PipelineShaderStageCreateInfo> shaderStages;
		std::vector<vk::UniqueShaderModule> shaderModules;
	};

	//// bool hasDescriptor{ false };

	// vk::UniquePipeline pipeline;
	// vk::DescriptorSet descSet;

	// UniquePtr<vl::RBuffer> uboBuf;
	// vk::UniquePipeline depthPipeline;

	UniquePtr<vl::RDescriptorLayout> descLayout;

	vk::UniquePipelineLayout gbufferPipelineLayout;
	vk::UniquePipelineLayout depthPipelineLayout;

	vk::UniqueShaderModule gbufferFragModule;
	vk::UniqueShaderModule depthFragModule;

	bool hasDescriptorSet{ false };
	MaterialArchetype::Gpu(PodHandle<MaterialArchetype> podHandle);

	std::vector<vk::PipelineShaderStageCreateInfo> gbufferShaderStages;
	std::vector<vk::PipelineShaderStageCreateInfo> depthShaderStages;

	// WIP:
	// PassInfo gbuffer;
	// PassInfo depth;

	void Update(const AssetUpdateInfo& info) override final;
};
