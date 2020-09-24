#pragma once
#include "rendering/wrappers/Buffer.h"

namespace vl {
struct GpuGeometryGroup;

struct RAccelerationStructure {

	[[nodiscard]] vk::AccelerationStructureKHR handle() const { return uHandle.get(); }
	[[nodiscard]] vk::DeviceMemory memory() const { return uMemory.get(); }

	// Only required for vk::AccelerationStructureInstanceKHR
	// TODO: Finish this instead of device addresses
	// uint64 getVkUint() { return static_cast<uint64>(handle.get().operator VkAccelerationStructureKHR()); }


protected:
	vk::UniqueAccelerationStructureKHR uHandle;
	vk::UniqueDeviceMemory uMemory;

	RBuffer scratchBuffer;

	void AllocateMemory();
};
} // namespace vl
