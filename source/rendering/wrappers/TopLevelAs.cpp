#include "pch.h"
#include "TopLevelAs.h"

#include "rendering/Device.h"
#include "rendering/scene/SceneGeometry.h"
#include "rendering/assets/GpuMesh.h"

namespace {
vk::AccelerationStructureInstanceKHR AsInstanceToVkGeometryInstanceKHR(const vl::AsInstance& instance)
{


	vk::AccelerationStructureInstanceKHR gInst{};
	// The matrices for the instance transforms are row-major, instead of
	// column-major in the rest of the application
	glm::mat4 transp = glm::transpose(instance.transform);
	// The gInst.transform value only contains 12 values, corresponding to a 4x3
	// matrix, hence saving the last row that is anyway always (0,0,0,1). Since
	// the matrix is row-major, we simply copy the first 12 values of the
	// original 4x4 matrix
	memcpy(&gInst.transform, &transp, sizeof(gInst.transform));
	gInst
		.setInstanceCustomIndex(instance.id) //
		.setInstanceShaderBindingTableRecordOffset(instance.hitGroupId)
		.setFlags(instance.flags)
		.setAccelerationStructureReference(instance.blasAddress);
	// gInst.mask = instance.mask; WIP: what is this?

	return gInst;
}
} // namespace

namespace vl {
TopLevelAs::TopLevelAs(const std::vector<SceneGeometry*>& geoms)
{
	for (int i = 0, j = 0; i < static_cast<int>(geoms.size()); i++) {
		if (geoms[i] && !geoms[i]->mesh.IsDefault()) {

			AsInstance inst{};
			inst.transform = geoms[i]->transform; // Position of the instance
			inst.id = j;                          // gl_InstanceID
			inst.blasAddress = geoms[i]->mesh.Lock().blas.GetAddress();
			inst.hitGroupId = 0; // We will use the same hit group for all objects
			inst.flags = vk::GeometryInstanceFlagBitsKHR::eTriangleFacingCullDisable; // WIP:
			instances.emplace_back(inst);
			++j;
		}
	}

	vk::AccelerationStructureCreateGeometryTypeInfoKHR geometryCreate{};
	geometryCreate
		.setGeometryType(vk::GeometryTypeKHR::eInstances) //
		.setMaxPrimitiveCount(static_cast<uint32>(instances.size()))
		.setAllowsTransforms(VK_TRUE);

	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo
		.setType(vk::AccelerationStructureTypeKHR::eTopLevel) //
		.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace)
		.setMaxGeometryCount(1u)
		.setPGeometryInfos(&geometryCreate);


	handle = Device->createAccelerationStructureKHRUnique(asCreateInfo);
	DEBUG_NAME(handle, "Scene Tlas");

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
	}

	// Compute the amount of scratch memory required by the acceleration structure builder
	vk::AccelerationStructureMemoryRequirementsInfoKHR asMemReqsInfo{};
	asMemReqsInfo
		.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch) //
		.setAccelerationStructure(handle.get())
		.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice);

	auto reqMem = Device->getAccelerationStructureMemoryRequirementsKHR(asMemReqsInfo);
	auto scratchBufferSize = reqMem.memoryRequirements.size;

	// Allocate the scratch memory
	RBuffer scratchBuffer{ scratchBufferSize,
		vk::BufferUsageFlagBits::eRayTracingKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	vk::BufferDeviceAddressInfo bufferInfo{};
	bufferInfo.setBuffer(scratchBuffer);

	// For each instance, build the corresponding instance descriptor
	std::vector<vk::AccelerationStructureInstanceKHR> geometryInstances;
	geometryInstances.reserve(instances.size());
	for (const auto& inst : instances) {
		geometryInstances.push_back(AsInstanceToVkGeometryInstanceKHR(inst));
	}


	// Building the TLAS


	// Create a buffer holding the actual instance data for use by the AS
	// builder
	vk::DeviceSize instanceDescsSizeInBytes = instances.size() * sizeof(VkAccelerationStructureInstanceKHR);

	// Allocate the instance buffer and copy its contents from host to device
	// memory
	RBuffer instBuffer{ instanceDescsSizeInBytes,
		vk::BufferUsageFlagBits::eRayTracingKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };
	instBuffer.UploadData(geometryInstances.data(), instanceDescsSizeInBytes);
	DEBUG_NAME(vk::Buffer(instBuffer), "TLASInstances");

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->computeCmdBuffer.begin(beginInfo);

	// Make sure the copy of the instance buffer are copied before triggering the
	// acceleration structure build
	vk::MemoryBarrier barrier{};
	barrier
		.setSrcAccessMask(vk::AccessFlagBits::eTransferWrite) //
		.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR);

	Device->computeCmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
		vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, vk::DependencyFlags{ 0 }, std::array{ barrier }, {},
		{});

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setCommandBufferCount(1u) //
		.setPCommandBuffers(&Device->computeCmdBuffer);

	Device->computeQueue.submit(1u, &submitInfo, {});

	// Build the TLAS
	vk::AccelerationStructureGeometryDataKHR geometry{};
	geometry.instances
		.setArrayOfPointers(VK_FALSE) //
		.data.setDeviceAddress(instBuffer.GetAddress());

	vk::AccelerationStructureGeometryKHR topASGeometry{};
	topASGeometry
		.setGeometryType(vk::GeometryTypeKHR::eInstances) //
		.setGeometry(geometry);


	const vk::AccelerationStructureGeometryKHR* pGeometry = &topASGeometry;
	vk::AccelerationStructureBuildGeometryInfoKHR topASInfo{};
	topASInfo
		.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace) //
		.setUpdate(VK_FALSE)
		.setSrcAccelerationStructure({})
		.setDstAccelerationStructure(handle.get())
		.setGeometryArrayOfPointers(VK_FALSE)
		.setGeometryCount(1u)
		.setPpGeometries(&pGeometry)
		.scratchData.setDeviceAddress(scratchBuffer.GetAddress());

	// Build Offsets info: n instances
	vk::AccelerationStructureBuildOffsetInfoKHR buildOffsetInfo{ static_cast<uint32_t>(instances.size()), 0, 0, 0 };
	std::vector<const vk::AccelerationStructureBuildOffsetInfoKHR*> pBuildOffset;
	pBuildOffset.push_back(&buildOffsetInfo);

	// Build the TLAS
	Device->computeCmdBuffer.buildAccelerationStructureKHR(1u, &topASInfo, pBuildOffset.data());


	Device->computeQueue.waitIdle();

	Device->waitIdle();
}
} // namespace vl
