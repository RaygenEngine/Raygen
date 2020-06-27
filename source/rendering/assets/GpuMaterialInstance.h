#pragma once
#include "assets/pods/MaterialInstance.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuMaterialArchetype.h"
#include "rendering/wrappers/RBuffer.h"

#include <vulkan/vulkan.hpp>


struct MaterialInstance::Gpu : public vl::GpuAssetTemplate<MaterialInstance> {
	vl::GpuHandle<MaterialArchetype> archetype;

	vk::UniquePipeline gbufferPipeline;
	vk::UniquePipeline depthPipeline;

	UniquePtr<vl::RBuffer> uboBuf;
	vk::DescriptorSet descSet;

	bool hasDescriptorSet{ false };
	MaterialInstance::Gpu(PodHandle<MaterialInstance> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
