#pragma once
#include "assets/pods/MaterialInstance.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/assets/GpuMaterialArchetype.h"

#include <vulkan/vulkan.hpp>


struct GpuMaterialInstance : public vl::GpuAssetTemplate<MaterialInstance> {
	vl::GpuHandle<MaterialArchetype> archetype;

	UniquePtr<vl::RBuffer> uboBuf;
	vk::DescriptorSet descSet;

	bool hasDescriptorSet{ false };
	GpuMaterialInstance(PodHandle<MaterialInstance> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
