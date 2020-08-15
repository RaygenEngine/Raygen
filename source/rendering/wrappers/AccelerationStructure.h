#pragma once
#include "rendering/wrappers/Buffer.h"

namespace vl {
struct GpuGeometryGroup;

struct RAccelerationStructure {

	RAccelerationStructure() = default;

	operator vk::AccelerationStructureKHR() const { return handle.get(); }
	operator bool() const { return operator vk::AccelerationStructureKHR(); }
	[[nodiscard]] vk::DeviceAddress GetAddress() const noexcept;

protected:
	vk::UniqueAccelerationStructureKHR handle;
	vk::UniqueDeviceMemory memory;

	RBuffer scratchBuffer;

	void AllocateMemory();
};
} // namespace vl
