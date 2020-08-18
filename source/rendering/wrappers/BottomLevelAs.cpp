#include "pch.h"
#include "BottomLevelAs.h"

#include "assets/shared/GeometryShared.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/Device.h"


namespace vl {


BottomLevelAs::BottomLevelAs(size_t vertexStride, const RBuffer& combinedVertexBuffer,
	const RBuffer& combinedIndexBuffer,
	GpuGeometryGroup& gg, //
	vk::BuildAccelerationStructureFlagsKHR buildFlags)
{
	std::vector<vk::AccelerationStructureCreateGeometryTypeInfoKHR> asCreateGeomInfos{};
	std::vector<vk::AccelerationStructureGeometryKHR> asGeoms{};
	std::vector<vk::AccelerationStructureBuildOffsetInfoKHR> asBuildOffsetInfos{};


	// Setting up the creation info of acceleration structure
	vk::AccelerationStructureCreateGeometryTypeInfoKHR asCreate{};
	asCreate
		.setGeometryType(vk::GeometryTypeKHR::eTriangles) //
		.setIndexType(vk::IndexType::eUint32)
		.setVertexFormat(vk::Format::eR32G32B32Sfloat)
		.setMaxPrimitiveCount(gg.indexCount / 3)
		.setMaxVertexCount(gg.vertexCount)
		.setAllowsTransforms(VK_FALSE); // No adding transformation matrices

	// Building part
	auto vertexAddress = combinedVertexBuffer.GetAddress();
	auto indexAddress = combinedIndexBuffer.GetAddress();

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
	offset.setPrimitiveCount(asCreate.maxPrimitiveCount)
		.setPrimitiveOffset(gg.indexBufferOffset)

		.setFirstVertex(gg.indexOffset) //
		.setTransformOffset(0);

	asGeoms.emplace_back(asGeom);
	asCreateGeomInfos.emplace_back(asCreate);
	asBuildOffsetInfos.emplace_back(offset);

	Build(buildFlags, asCreateGeomInfos, asGeoms, asBuildOffsetInfos);
}

void BottomLevelAs::Build(vk::BuildAccelerationStructureFlagsKHR buildFlags,
	const std::vector<vk::AccelerationStructureCreateGeometryTypeInfoKHR>& asCreateGeomInfos,
	const std::vector<vk::AccelerationStructureGeometryKHR>& asGeoms,
	const std::vector<vk::AccelerationStructureBuildOffsetInfoKHR>& asBuildOffsetInfos)
{


	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo
		.setFlags(buildFlags) //
		.setMaxGeometryCount(static_cast<uint32>(asCreateGeomInfos.size()))
		.setPGeometryInfos(asCreateGeomInfos.data())
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);

	handle = Device->createAccelerationStructureKHRUnique(asCreateInfo);

	DEBUG_NAME(handle, "Blas");

	AllocateMemory();

	// Compute the amount of scratch memory required by the acceleration structure builder
	vk::AccelerationStructureMemoryRequirementsInfoKHR asMemReqsInfo{};
	asMemReqsInfo
		.setType(vk::AccelerationStructureMemoryRequirementsTypeKHR::eBuildScratch) //
		.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice)
		.setAccelerationStructure(handle.get());

	auto memReqs = Device->getAccelerationStructureMemoryRequirementsKHR(asMemReqsInfo);

	// Acceleration structure build requires some scratch space to store temporary information
	const auto scratchBufferSize = memReqs.memoryRequirements.size;

	// TODO: use a single scratch buffer based on the maximum requirements of the scene BVH\
	// approach: create a decent sized one (in Device probably)
	// access it from within AS construction
	// if memReqs.size > Device->scratchBufferAs.capab.size then we need to update it (rebuild it)
	scratchBuffer
		= { scratchBufferSize, vk::BufferUsageFlagBits::eRayTracingKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
			  vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	const vk::AccelerationStructureGeometryKHR* pGeometry = asGeoms.data();
	vk::AccelerationStructureBuildGeometryInfoKHR asBuildGeomInfo{};
	asBuildGeomInfo
		.setFlags(vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace) //
		.setUpdate(VK_FALSE)
		.setSrcAccelerationStructure({})
		.setDstAccelerationStructure(handle.get())
		.setGeometryCount(static_cast<uint32>(asGeoms.size()))
		.setGeometryArrayOfPointers(VK_FALSE)
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
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
	Device->computeQueue.waitIdle(); // NEXT:
}
} // namespace vl
