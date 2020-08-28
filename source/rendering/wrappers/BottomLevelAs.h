#pragma once
#include "rendering/wrappers/AccelerationStructure.h"

namespace vl {
struct GpuGeometryGroup;

struct BottomLevelAs : RAccelerationStructure {

	BottomLevelAs() = default;
	BottomLevelAs(size_t vertexStride, const RBuffer& combinedVertexBuffer, const RBuffer& combinedIndexBuffer,
		GpuGeometryGroup& ggg, vk::BuildAccelerationStructureFlagsKHR buildFlags);

private:
	void Build(vk::BuildAccelerationStructureFlagsKHR buildFlags,
		const std::vector<vk::AccelerationStructureCreateGeometryTypeInfoKHR>& asCreateGeomInfos,
		const std::vector<vk::AccelerationStructureGeometryKHR>& asGeoms,
		const std::vector<vk::AccelerationStructureBuildOffsetInfoKHR>& asBuildOffsetInfos);
};
} // namespace vl
