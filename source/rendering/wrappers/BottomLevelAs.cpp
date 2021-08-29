#include "BottomLevelAs.h"

#include "assets/pods/MaterialArchetype.h"
#include "rendering/Device.h"
#include "rendering/assets/GpuMaterialArchetype.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/wrappers/CmdBuffer.h"

namespace vl {


BottomLevelAs::BottomLevelAs(size_t vertexStride, const RBuffer& combinedVertexBuffer,
	const RBuffer& combinedIndexBuffer,
	GpuGeometryGroup& gg, //
	vk::BuildAccelerationStructureFlagsKHR buildFlags)
{
	std::vector<vk::AccelerationStructureGeometryKHR> asGeoms{};
	std::vector<vk::AccelerationStructureBuildRangeInfoKHR> asBuildRangeInfos{};
	std::vector<uint32> maxPrimitiveCounts{};

	// Building part
	auto vertexAddress = Device->getBufferAddress(combinedVertexBuffer.handle());
	auto indexAddress = Device->getBufferAddress(combinedIndexBuffer.handle());

	vk::AccelerationStructureGeometryTrianglesDataKHR triangles{};
	triangles
		.setVertexFormat(vk::Format::eR32G32B32Sfloat) //
		.setVertexData(vertexAddress)
		.setVertexStride(vertexStride)
		.setIndexType(vk::IndexType::eUint32)
		.setIndexData(indexAddress)
		.setMaxVertex(gg.vertexCount - 1) // TODO:
		.setTransformData({});

	// Setting up the build info of the acceleration
	vk::AccelerationStructureGeometryKHR asGeom{};
	asGeom
		.setGeometryType(vk::GeometryTypeKHR::eTriangles) //
		.setFlags(vk::GeometryFlagBitsKHR::eOpaque)
		.geometry.setTriangles(triangles);

	PodHandle<MaterialInstance> matInst{ gg.material.Lock().podUid };
	PodHandle<MaterialArchetype> matArch{ gg.material.Lock().archetype.Lock().podUid };
	auto& cl = matArch.Lock()->descriptorSetLayout.uboClass;
	auto prp = cl.GetPropertyByName(std::string("doubleSided")); // DOC:

	if (prp) {
		doubleSided = prp->GetRef<bool>((void*)matInst.Lock()->descriptorSet.uboData.data());

		if (doubleSided) {
			asGeom.setFlags(vk::GeometryFlagBitsKHR::eNoDuplicateAnyHitInvocation);
		}
	}

	const auto maxPrimitiveCount = gg.indexCount / 3;

	// The primitive itself
	vk::AccelerationStructureBuildRangeInfoKHR asBuildRange{};
	asBuildRange
		.setPrimitiveCount(maxPrimitiveCount) //
		.setPrimitiveOffset(gg.indexBufferOffset)
		.setFirstVertex(gg.indexOffset)
		.setTransformOffset(0);

	asGeoms.emplace_back(asGeom);
	asBuildRangeInfos.emplace_back(asBuildRange);
	maxPrimitiveCounts.emplace_back(maxPrimitiveCount);

	Build(buildFlags, asGeoms, asBuildRangeInfos, maxPrimitiveCounts);
}

BottomLevelAs::BottomLevelAs(const RBuffer& combinedVertexBuffer, uint32 vertexCount,
	const RBuffer& combinedIndexBuffer, uint32 indexCount, vk::BuildAccelerationStructureFlagsKHR buildFlags)
{
	std::vector<vk::AccelerationStructureGeometryKHR> asGeoms{};
	std::vector<vk::AccelerationStructureBuildRangeInfoKHR> asBuildRangeInfos{};
	std::vector<uint32> maxPrimitiveCounts{};

	// Building part
	auto vertexAddress = Device->getBufferAddress(combinedVertexBuffer.handle());
	auto indexAddress = Device->getBufferAddress(combinedIndexBuffer.handle());

	vk::AccelerationStructureGeometryTrianglesDataKHR triangles{};
	triangles
		.setVertexFormat(vk::Format::eR32G32B32Sfloat) //
		.setVertexData(vertexAddress)
		.setVertexStride(sizeof(glm::vec3))
		.setIndexType(vk::IndexType::eUint32)
		.setIndexData(indexAddress)
		.setMaxVertex(vertexCount - 1) // TODO:
		.setTransformData({});

	// Setting up the build info of the acceleration
	vk::AccelerationStructureGeometryKHR asGeom{};
	asGeom
		.setGeometryType(vk::GeometryTypeKHR::eTriangles) //
		.setFlags(vk::GeometryFlagBitsKHR::eOpaque)
		.geometry.setTriangles(triangles);

	const auto maxPrimitiveCount = indexCount / 3;

	// The primitive itself
	vk::AccelerationStructureBuildRangeInfoKHR asBuildRange{};
	asBuildRange
		.setPrimitiveCount(maxPrimitiveCount) //
		.setPrimitiveOffset(0u)
		.setFirstVertex(0u)
		.setTransformOffset(0);

	asGeoms.emplace_back(asGeom);
	asBuildRangeInfos.emplace_back(asBuildRange);
	maxPrimitiveCounts.emplace_back(maxPrimitiveCount);

	Build(buildFlags, asGeoms, asBuildRangeInfos, maxPrimitiveCounts);
}

void BottomLevelAs::Build(vk::BuildAccelerationStructureFlagsKHR buildFlags,
	const std::vector<vk::AccelerationStructureGeometryKHR>& asGeoms,
	const std::vector<vk::AccelerationStructureBuildRangeInfoKHR>& asBuildRangeInfos,
	const std::vector<uint32>& maxPrimitiveCounts)
{
	const vk::AccelerationStructureGeometryKHR* pGeometry = asGeoms.data();
	vk::AccelerationStructureBuildGeometryInfoKHR asBuildGeomInfo{};
	asBuildGeomInfo
		.setFlags(buildFlags) //
		.setGeometryCount(static_cast<uint32>(asGeoms.size()))
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
		.setPpGeometries(&pGeometry)
		.setMode(vk::BuildAccelerationStructureModeKHR::eBuild);

	auto asBuildSizes = Device->getAccelerationStructureBuildSizesKHR(
		vk::AccelerationStructureBuildTypeKHR::eDevice, asBuildGeomInfo, maxPrimitiveCounts);

	// TODO: use a single scratch buffer based on the maximum requirements of the scene BVH\
	// approach: create a decent sized one (in Device probably)
	// access it from within AS construction
	// if memReqs.size > Device->scratchBufferAs.capab.size then we need to update it (rebuild it)
	scratchBuffer = { asBuildSizes.buildScratchSize,
		vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR
			| vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	asBuffer = { asBuildSizes.accelerationStructureSize, vk::BufferUsageFlagBits::eAccelerationStructureStorageKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal };

	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo
		.setBuffer(asBuffer.handle()) // is the buffer on which the acceleration structure will be stored.
		.setOffset(0u) // is an offset in bytes from the base address of the buffer at which the acceleration structure
					   // will be stored, and must be a multiple of 256.
		.setSize(asBuildSizes.accelerationStructureSize) // is the size required for the acceleration structure.
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);

	uHandle = Device->createAccelerationStructureKHRUnique(asCreateInfo);

	DEBUG_NAME(uHandle, "Blas");

	asBuildGeomInfo
		.setScratchData(scratchBuffer.address()) //
		.setSrcAccelerationStructure({})
		.setDstAccelerationStructure(uHandle.get());

	// Pointers of offset
	std::vector<const vk::AccelerationStructureBuildRangeInfoKHR*> pBuildRange(asBuildRangeInfos.size());
	for (size_t i = 0; i < asBuildRangeInfos.size(); ++i) {
		pBuildRange[i] = &asBuildRangeInfos[i];
	}

	{
		ScopedOneTimeSubmitCmdBuffer<Compute> cmdBuffer{};

		cmdBuffer.buildAccelerationStructuresKHR(asBuildGeomInfo, pBuildRange);

		vk::MemoryBarrier memoryBarrier{};
		memoryBarrier
			.setSrcAccessMask(vk::AccessFlagBits::eAccelerationStructureWriteKHR) //
			.setDstAccessMask(vk::AccessFlagBits::eAccelerationStructureReadKHR);

		cmdBuffer.pipelineBarrier(vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR,
			vk::PipelineStageFlagBits::eAccelerationStructureBuildKHR, vk::DependencyFlags{ 0 }, memoryBarrier, {}, {});
		// TODO: compacting - for those that don't update often
	}
}

} // namespace vl
