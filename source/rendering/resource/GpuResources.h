#pragma once
#include "rendering/resource/DescPoolAllocator.h"

namespace vl {
inline struct GpuResources_ {
	DescPoolAllocator descPools;
	DescriptorLayout imageDebugDescLayout;

	GpuResources_();
} * GpuResources{};
} // namespace vl
