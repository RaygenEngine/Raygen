#pragma once
#include "rendering/resource/DescPoolAllocator.h"

namespace vl {
inline struct GpuResources_ {
	DescPoolAllocator descPools;
	RDescriptorLayout imageDebugDescLayout;

	GpuResources_();
} * GpuResources{};
} // namespace vl
