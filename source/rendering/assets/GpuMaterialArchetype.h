#pragma once
#include "assets/pods/MaterialArchetype.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/assets/GpuSampler.h"
#include "rendering/wrappers/RBuffer.h"

#include <vulkan/vulkan.hpp>

struct MaterialArchetype::Gpu : public vl::GpuAssetTemplate<MaterialArchetype> {
	// vk::UniquePipelineLayout plLayout;
	// vk::UniquePipeline pipeline;

	//// bool hasDescriptor{ false };
	// UniquePtr<vl::RDescriptorLayout> descLayout;
	// vk::DescriptorSet descSet;

	// UniquePtr<vl::RBuffer> uboBuf;

	// vk::UniqueShaderModule fragModule;

	// vk::UniquePipelineLayout depthPlLayout;
	// vk::UniquePipeline depthPipeline;

	// vk::UniqueShaderModule depthFragModule;

	// bool hasDescriptorSet{ false };
	MaterialArchetype::Gpu(PodHandle<MaterialArchetype> podHandle);

	// void Update(const AssetUpdateInfo& info) override final;
};
