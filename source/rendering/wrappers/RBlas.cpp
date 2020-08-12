#include "pch.h"
#include "RBlas.h"

#include "assets/shared/GeometryShared.h"
#include "rendering/Device.h"
#include "rendering/wrappers/RBuffer.h"

namespace vl {

RBlas::RBlas(size_t vertexStride, const std::vector<GpuGeometryGroup>& gggs, //
	vk::BuildAccelerationStructureFlagsKHR buildFlags)
{
	std::vector<vk::AccelerationStructureCreateGeometryTypeInfoKHR> asCreateGeomInfos{};
	std::vector<vk::AccelerationStructureGeometryKHR> asGeoms{};
	std::vector<vk::AccelerationStructureBuildOffsetInfoKHR> asBuildOffsetInfos{};

	for (const auto& ggg : gggs) {
		// Setting up the creation info of acceleration structure
		vk::AccelerationStructureCreateGeometryTypeInfoKHR asCreate{};
		asCreate
			.setGeometryType(vk::GeometryTypeKHR::eTriangles) //
			.setIndexType(vk::IndexType::eUint32)
			.setVertexFormat(vk::Format::eR32G32B32Sfloat)
			.setMaxPrimitiveCount(ggg.indexCount / 3)
			.setMaxVertexCount(ggg.vertexCount)
			.setAllowsTransforms(VK_FALSE); // No adding transformation matrices

		// Building part
		auto vertexAddress = ggg.vertexBuffer.GetAddress();
		auto indexAddress = ggg.indexBuffer.GetAddress();

		vk::AccelerationStructureGeometryTrianglesDataKHR triangles{};
		triangles
			.setVertexFormat(asCreate.vertexFormat) //
			.setVertexData(vertexAddress)
			.setVertexStride(vertexStride)
			.setIndexType(asCreate.indexType)
			.setIndexData(indexAddress)
			.setTransformData({});

		// Setting up the build info of the acceleration
		vk::AccelerationStructureGeometryKHR asGeom{};
		asGeom
			.setGeometryType(asCreate.geometryType) //
			.setFlags(vk::GeometryFlagBitsKHR::eOpaque)
			.geometry.setTriangles(triangles);

		// The primitive itself
		vk::AccelerationStructureBuildOffsetInfoKHR offset{};
		offset
			.setFirstVertex(0) //
			.setPrimitiveCount(asCreate.maxPrimitiveCount)
			.setPrimitiveOffset(0)
			.setTransformOffset(0);

		asGeoms.emplace_back(asGeom);
		asCreateGeomInfos.emplace_back(asCreate);
		asBuildOffsetInfos.emplace_back(offset);
	}


	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo
		.setFlags(buildFlags) //
		.setMaxGeometryCount(static_cast<uint32>(asCreateGeomInfos.size()))
		.setPGeometryInfos(asCreateGeomInfos.data())
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);

	m_handle = Device->createAccelerationStructureKHRUnique(asCreateInfo);

	{
		vk::AccelerationStructureMemoryRequirementsInfoKHR memInfo{};
		memInfo.setAccelerationStructure(m_handle.get())
			.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice)
			.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eObject);

		auto asMemReqs = Device->getAccelerationStructureMemoryRequirementsKHR(memInfo);

		VkMemoryAllocateFlagsInfo memFlagInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		memFlagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;

		// 3. Allocate memory
		VkMemoryAllocateInfo memAlloc{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
		memAlloc.allocationSize = asMemReqs.memoryRequirements.size;
		memAlloc.memoryTypeIndex = Device->FindMemoryType(
			asMemReqs.memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);

		m_memory = Device->allocateMemoryUnique(memAlloc);

		// 4. Bind memory with acceleration structure
		VkBindAccelerationStructureMemoryInfoKHR bind{ VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR };
		bind.accelerationStructure = m_handle.get();
		bind.memory = m_memory.get();
		bind.memoryOffset = 0;

		Device->bindAccelerationStructureMemoryKHR({ bind });
	}


	vk::AccelerationStructureMemoryRequirementsInfoKHR asMemReqsInfo{};
	asMemReqsInfo.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch)
		.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice) //
		.setAccelerationStructure(m_handle.get());

	auto memReqs = Device->getAccelerationStructureMemoryRequirementsKHR(asMemReqsInfo);

	// Acceleration structure build requires some scratch space to store temporary information
	const vk::DeviceSize scratchBufferSize = memReqs.memoryRequirements.size;
	LOG_REPORT("Size reqs {}MB", scratchBufferSize / 1000000.0);

	RBuffer scratchBuffer{ scratchBufferSize,
		vk::BufferUsageFlagBits::eRayTracingKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	vk::AccelerationStructureGeometryKHR* pGeometry = asGeoms.data();

	vk::AccelerationStructureBuildGeometryInfoKHR asBuildGeomInfo{};
	asBuildGeomInfo
		.setDstAccelerationStructure(m_handle.get()) //
		.setSrcAccelerationStructure({})
		.setUpdate(VK_FALSE)
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
		.setGeometryCount(static_cast<uint32>(asGeoms.size()))
		.setGeometryArrayOfPointers(VK_FALSE)
		.setPpGeometries(&pGeometry)
		.setScratchData(scratchBuffer.GetAddress());

	// Pointers of offset
	std::vector<const vk::AccelerationStructureBuildOffsetInfoKHR*> pBuildOffset(asBuildOffsetInfos.size());
	for (size_t i = 0; i < asBuildOffsetInfos.size(); ++i) {
		pBuildOffset[i] = &asBuildOffsetInfos[i];
	}

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->computeCmdBuffer.begin(beginInfo);

	Device->computeCmdBuffer.buildAccelerationStructureKHR(1u, &asBuildGeomInfo, pBuildOffset.data());

	vk::MemoryBarrier memoryBarrier{};
	memoryBarrier
		.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR) //
		.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureReadKHR);


	Device->computeCmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
		vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, vk::DependencyFlags{ 0 },
		std::array{ memoryBarrier }, {}, {});

	Device->computeCmdBuffer.end();

	// WIP: compacting

	vk::SubmitInfo submitInfo{};
	submitInfo
		.setCommandBufferCount(1u) //
		.setPCommandBuffers(&Device->computeCmdBuffer);

	Device->computeQueue.submit(1u, &submitInfo, {});
	Device->computeQueue.waitIdle();

	Device->waitIdle();
}


} // namespace vl
