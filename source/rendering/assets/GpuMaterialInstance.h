#pragma once
#include "assets/pods/MaterialInstance.h"
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/wrappers/Buffer.h"


namespace vl {
struct GpuMaterialInstance : public GpuAssetTemplate<MaterialInstance> {
	GpuHandle<MaterialArchetype> archetype;

	RBuffer uboBuf;
	vk::DescriptorSet descSet;

	bool hasDescriptorSet{ false };
	GpuMaterialInstance(PodHandle<MaterialInstance> podHandle);

	void Update(const AssetUpdateInfo& info) override final;
};
} // namespace vl
