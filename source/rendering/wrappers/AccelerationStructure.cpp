#include "pch.h"
#include "AccelerationStructure.h"

#include "rendering/Device.h"

namespace vl {

vk::DeviceAddress RAccelerationStructure::GetAddress() const noexcept
{
	return Device->getAccelerationStructureAddressKHR(vk::AccelerationStructureDeviceAddressInfoKHR{ handle.get() });
}

void RAccelerationStructure::AllocateMemory()
{
	vk::AccelerationStructureMemoryRequirementsInfoKHR memInfo{};
	memInfo
		.setAccelerationStructure(handle.get()) //
		.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice)
		.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eObject);

	auto asMemReqs = Device->getAccelerationStructureMemoryRequirementsKHR(memInfo);

	vk::MemoryAllocateFlagsInfo memFlagInfo{};
	memFlagInfo.setFlags(vk::MemoryAllocateFlagBits::eDeviceAddress);

	// 3. Allocate memory
	vk::MemoryAllocateInfo memAlloc{};
	memAlloc
		.setAllocationSize(asMemReqs.memoryRequirements.size) //
		.setMemoryTypeIndex(Device->FindMemoryType(
			asMemReqs.memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal));

	memory = Device->allocateMemoryUnique(memAlloc);

	// 4. Bind memory with acceleration structure
	vk::BindAccelerationStructureMemoryInfoKHR bind{};
	bind.setAccelerationStructure(handle.get()) //
		.setMemory(memory.get())
		.setMemoryOffset(0);

	Device->bindAccelerationStructureMemoryKHR({ bind });

	LOG_REPORT("Accel size reqs {}KB", asMemReqs.memoryRequirements.size / 1000.0);
}

} // namespace vl
