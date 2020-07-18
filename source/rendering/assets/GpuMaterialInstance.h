#pragma once
#include "assets/pods/MaterialInstance.h"
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/assets/GpuMaterialArchetype.h"

#include <vulkan/vulkan.hpp>

namespace vl {
struct GpuMaterialInstance : public GpuAssetTemplate<MaterialInstance> {
	GpuHandle<MaterialArchetype> archetype;

	UniquePtr<RBuffer> uboBuf;
	vk::DescriptorSet descSet;

	bool hasDescriptorSet{ false };
	GpuMaterialInstance(PodHandle<MaterialInstance> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
