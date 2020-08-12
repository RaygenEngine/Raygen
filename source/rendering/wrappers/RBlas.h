#pragma once
#include "rendering/assets/GpuMesh.h"

namespace vl {

struct RBlas {

	RBlas(size_t vertexStride, const std::vector<GpuGeometryGroup>& gggs,
		vk::BuildAccelerationStructureFlagsKHR buildFlags = {});

private:
	vk::UniqueAccelerationStructureKHR m_handle;
	vk::UniqueDeviceMemory m_memory;
};
} // namespace vl
