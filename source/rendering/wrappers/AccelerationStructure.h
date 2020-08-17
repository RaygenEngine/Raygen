#pragma once
#include "rendering/wrappers/Buffer.h"

namespace vl {
struct GpuGeometryGroup;

struct RAccelerationStructure {

	RAccelerationStructure() = default;

	operator vk::AccelerationStructureKHR() const { return handle.get(); }
	operator bool() const { return operator vk::AccelerationStructureKHR(); }
	[[nodiscard]] vk::DeviceAddress GetAddress() const noexcept;

	vk::AccelerationStructureKHR get() { return handle.get(); }

	// Only required for vk::AccelerationStructureInstanceKHR
	// WIP: Finish this instead of device addresses
	// uint64 getVkUint() { return static_cast<uint64>(handle.get().operator VkAccelerationStructureKHR()); }

protected:
	vk::UniqueAccelerationStructureKHR handle;
	vk::UniqueDeviceMemory memory;

	RBuffer scratchBuffer;

	void AllocateMemory();
};
} // namespace vl
