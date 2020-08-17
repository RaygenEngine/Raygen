#pragma once
#include "rendering/wrappers/AccelerationStructure.h"

namespace vl {
struct GpuGeometryGroup;

struct BottomLevelAs : RAccelerationStructure {

	BottomLevelAs() = default;

	// Multi Geometry Groups version
	BottomLevelAs(size_t vertexStride, const std::vector<GpuGeometryGroup>& gggs,
		vk::BuildAccelerationStructureFlagsKHR buildFlags = {});

	// Single Geometry group version
	BottomLevelAs::BottomLevelAs(
		size_t vertexStride, GpuGeometryGroup& ggg, vk::BuildAccelerationStructureFlagsKHR buildFlags);

private:
	void Build(vk::BuildAccelerationStructureFlagsKHR buildFlags,
		const std::vector<vk::AccelerationStructureCreateGeometryTypeInfoKHR>& asCreateGeomInfos,
		const std::vector<vk::AccelerationStructureGeometryKHR>& asGeoms,
		const std::vector<vk::AccelerationStructureBuildOffsetInfoKHR>& asBuildOffsetInfos);
};
} // namespace vl
