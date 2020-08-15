#pragma once
#include "rendering/wrappers/AccelerationStructure.h"

namespace vl {
struct GpuGeometryGroup;

struct BottomLevelAs : RAccelerationStructure {

	BottomLevelAs() = default;
	BottomLevelAs(size_t vertexStride, const std::vector<GpuGeometryGroup>& gggs,
		vk::BuildAccelerationStructureFlagsKHR buildFlags = {});
};
} // namespace vl
