#pragma once
#include "rendering/wrappers/AccelerationStructure.h"

namespace vl {
struct GpuGeometryGroup;

struct BottomLevelAs : public RAccelerationStructure {

	BottomLevelAs() = default;
	BottomLevelAs(size_t vertexStride, const RBuffer& combinedVertexBuffer, const RBuffer& combinedIndexBuffer,
		GpuGeometryGroup& ggg, vk::BuildAccelerationStructureFlagsKHR buildFlags);

	BottomLevelAs(const RBuffer& combinedVertexBuffer, uint32 vertexCount, const RBuffer& combinedIndexBuffer,
		uint32 indexCount, vk::BuildAccelerationStructureFlagsKHR buildFlags);

private:
	void Build(vk::BuildAccelerationStructureFlagsKHR buildFlags,
		const std::vector<vk::AccelerationStructureCreateGeometryTypeInfoKHR>& asCreateGeomInfos,
		const std::vector<vk::AccelerationStructureGeometryKHR>& asGeoms,
		const std::vector<vk::AccelerationStructureBuildOffsetInfoKHR>& asBuildOffsetInfos);
};
} // namespace vl
