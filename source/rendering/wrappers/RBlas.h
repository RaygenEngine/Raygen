#pragma once
#include "rendering/assets/GpuMesh.h"

namespace vl {

class RBlas {
	vk::UniqueAccelerationStructureKHR m_handle;
	vk::UniqueDeviceMemory m_memory;

public:
	RBlas(size_t vertexStride, const std::vector<GpuGeometryGroup>& gggs,
		vk::BuildAccelerationStructureFlagsKHR buildFlags = {});
};
} // namespace vl
