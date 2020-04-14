#pragma once
#include "assets/pods/Cubemap.h"
#include "rendering/assets/GpuAssetHandle.h"
#include "rendering/objects/Cubemap.h"

#include <vulkan/vulkan.hpp>

struct ::Cubemap::Gpu : public vl::GpuAssetBase {
	UniquePtr<vl::Cubemap> cubemap;

	Cubemap::Gpu(PodHandle<Cubemap> podHandle);
};
