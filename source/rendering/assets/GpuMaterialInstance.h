#pragma once
#include "assets/pods/MaterialInstance.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuSampler.h"
#include "rendering/wrappers/RBuffer.h"

#include <vulkan/vulkan.hpp>


struct MaterialInstance::Gpu : public vl::GpuAssetTemplate<MaterialInstance> {
	vk::UniquePipelineLayout plLayout;
	vk::UniquePipeline pipeline;

	UniquePtr<vl::RDescriptorLayout> descLayout;
	vk::DescriptorSet descSet;

	UniquePtr<vl::RBuffer> uboBuf;

	vk::UniqueShaderModule fragModule;

	vk::UniquePipelineLayout depthPlLayout;
	vk::UniquePipeline depthPipeline;

	vk::UniqueShaderModule depthFragModule;

	bool hasDescriptorSet{ false };
	MaterialInstance::Gpu(PodHandle<MaterialInstance> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
