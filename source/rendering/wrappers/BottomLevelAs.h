#pragma once
#include "rendering/wrappers/AccelerationStructure.h"

namespace vl {
struct GpuGeometryGroup;

struct BottomLevelAs : public RAccelerationStructure {

	// WIP:
	bool isMask{ false };

	BottomLevelAs() = default;
	BottomLevelAs(size_t vertexStride, const RBuffer& combinedVertexBuffer, const RBuffer& combinedIndexBuffer,
		GpuGeometryGroup& ggg, vk::BuildAccelerationStructureFlagsKHR buildFlags);

	BottomLevelAs(const RBuffer& combinedVertexBuffer, uint32 vertexCount, const RBuffer& combinedIndexBuffer,
		uint32 indexCount, vk::BuildAccelerationStructureFlagsKHR buildFlags);

private:
	void Build(vk::BuildAccelerationStructureFlagsKHR buildFlags,
		const std::vector<vk::AccelerationStructureGeometryKHR>& asGeoms,
		const std::vector<vk::AccelerationStructureBuildRangeInfoKHR>& asBuildRangeInfos,
		const std::vector<uint32>& maxPrimitiveCounts);
};
} // namespace vl
