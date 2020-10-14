#include "AccelerationStructure.h"

#include "rendering/Device.h"

namespace vl {
void RAccelerationStructure::AllocateMemory()
{
	vk::AccelerationStructureMemoryRequirementsInfoKHR memInfo{};
	memInfo
		.setAccelerationStructure(uHandle.get()) //
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

	uMemory = Device->allocateMemoryUnique(memAlloc);

	// 4. Bind memory with acceleration structure
	vk::BindAccelerationStructureMemoryInfoKHR bind{};
	bind.setAccelerationStructure(uHandle.get()) //
		.setMemory(uMemory.get())
		.setMemoryOffset(0);

	Device->bindAccelerationStructureMemoryKHR(bind);

	LOG_REPORT("Accel size reqs {}KB", asMemReqs.memoryRequirements.size / 1000.0);
}

} // namespace vl
