#include "pch.h"
#include "GpuMesh.h"

#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/wrappers/RBlas.h"
#include "rendering/wrappers/RBuffer.h"
#include "assets/AssetRegistry.h"
#include "rendering/Instance.h"

#define VK_KHR_ray_tracing
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>


using namespace vl;

GpuMesh::GpuMesh(PodHandle<Mesh> podHandle)
	: GpuAssetTemplate(podHandle)
{
	auto data = podHandle.Lock();

	Update({});
}
// struct AccelerationDedicatedKHR {
//	VkAccelerationStructureKHR accel{ VK_NULL_HANDLE };
//	VkDeviceMemory allocation{ VK_NULL_HANDLE };
//};
//// Bottom-level acceleration structure
// struct Blas {
//	AccelerationDedicatedKHR dedicated;
//
//	VkBuildAccelerationStructureFlagsKHR flags; // specifying additional parameters for acceleration structure
//												// builds
//	std::vector<VkAccelerationStructureCreateGeometryTypeInfoKHR>
//		asCreateGeometryInfo;                                   // specifies the shape of geometries that will be
//																// built into an acceleration structure
//	std::vector<VkAccelerationStructureGeometryKHR> asGeometry; // data used to build acceleration structure geometry
//	std::vector<VkAccelerationStructureBuildOffsetInfoKHR> asBuildOffsetInfo;
//};
//
//// Vector containing all the BLASes built and referenced by the TLAS
// std::vector<Blas> m_blas;
//
// AccelerationDedicatedKHR createAcceleration(VkAccelerationStructureCreateInfoKHR& accel_)
//{
//	AccelerationDedicatedKHR resultAccel;
//	// 1. Create the acceleration structure
//
//	VkDevice m_device = *Device;
//
//	vkCreateAccelerationStructureKHR(m_device, &accel_, nullptr, &resultAccel.accel);
//
//
//	// 2. Find memory requirements
//	VkAccelerationStructureMemoryRequirementsInfoKHR memInfo{
//		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR
//	};
//	memInfo.accelerationStructure = resultAccel.accel;
//	memInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
//	memInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
//	VkMemoryRequirements2 memReqs{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
//	vkGetAccelerationStructureMemoryRequirementsKHR(m_device, &memInfo, &memReqs);
//
//
//	VkMemoryAllocateFlagsInfo memFlagInfo{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
//	memFlagInfo.flags = VK_MEMORY_ALLOCATE_DEVICE_ADDRESS_BIT;
//
//	// 3. Allocate memory
//	VkMemoryAllocateInfo memAlloc{ VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO };
//	memAlloc.allocationSize = memReqs.memoryRequirements.size;
//	memAlloc.memoryTypeIndex = Device->pd->FindMemoryType(
//		memReqs.memoryRequirements.memoryTypeBits, vk::MemoryPropertyFlagBits::eDeviceLocal);
//	resultAccel.allocation = Device->allocateMemory(memAlloc);
//
//	// 4. Bind memory with acceleration structure
//	VkBindAccelerationStructureMemoryInfoKHR bind{ VK_STRUCTURE_TYPE_BIND_ACCELERATION_STRUCTURE_MEMORY_INFO_KHR };
//	bind.accelerationStructure = resultAccel.accel;
//	bind.memory = resultAccel.allocation;
//	bind.memoryOffset = 0;
//	vkBindAccelerationStructureMemoryKHR(m_device, 1, &bind);
//
//	return resultAccel;
//}
//
// void buildBlas(GpuMesh& mesh,
//	VkBuildAccelerationStructureFlagsKHR flags = VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR)
//{
//
//	auto& vgg = mesh.geometryGroups[0];
//
//	// Setting up the creation info of acceleration structure
//	vk::AccelerationStructureCreateGeometryTypeInfoKHR asCreate;
//	asCreate.setGeometryType(vk::GeometryTypeKHR::eTriangles);
//	asCreate.setIndexType(vk::IndexType::eUint32);
//	asCreate.setVertexFormat(vk::Format::eR32G32B32Sfloat);
//	asCreate.setMaxPrimitiveCount(vgg.indexCount / 3); // Nb triangles
//	asCreate.setMaxVertexCount(vgg.vertexCount);
//	asCreate.setAllowsTransforms(VK_FALSE); // No adding transformation matrices
//
//	// Building part
//	vk::DeviceAddress vertexAddress = Device->getBufferAddress({ vgg.vertexBuffer->m_handle.get() });
//	vk::DeviceAddress indexAddress = Device->getBufferAddress({ vgg.indexBuffer->m_handle.get() });
//
//	vk::AccelerationStructureGeometryTrianglesDataKHR triangles;
//	triangles.setVertexFormat(asCreate.vertexFormat);
//	triangles.setVertexData(vertexAddress);
//	triangles.setVertexStride(sizeof(Vertex));
//	triangles.setIndexType(asCreate.indexType);
//	triangles.setIndexData(indexAddress);
//	triangles.setTransformData({});
//
//	// Setting up the build info of the acceleration
//	vk::AccelerationStructureGeometryKHR asGeom;
//	asGeom.setGeometryType(asCreate.geometryType);
//	asGeom.setFlags(vk::GeometryFlagBitsKHR::eOpaque);
//	asGeom.geometry.setTriangles(triangles);
//
//	// The primitive itself
//	vk::AccelerationStructureBuildOffsetInfoKHR offset;
//	offset.setFirstVertex(0);
//	offset.setPrimitiveCount(asCreate.maxPrimitiveCount);
//	offset.setPrimitiveOffset(0);
//	offset.setTransformOffset(0);
//
//	// Our blas is only one geometry, but could be made of many geometries
//	Blas blas;
//	blas.asGeometry.emplace_back(asGeom);
//	blas.asCreateGeometryInfo.emplace_back(asCreate);
//	blas.asBuildOffsetInfo.emplace_back(offset);
//
//	m_blas.emplace_back(blas);
//
//	VkDeviceSize maxScratch{ 0 }; // Largest scratch buffer for our BLAS
//
//	// Is compaction requested?
//	bool doCompaction = false; // = (flags & VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR)
//							   //== VK_BUILD_ACCELERATION_STRUCTURE_ALLOW_COMPACTION_BIT_KHR;
//	std::vector<VkDeviceSize> originalSizes;
//	originalSizes.resize(m_blas.size());
//
//	// Iterate over the groups of geometries, creating one BLAS for each group
//	int idx{ 0 };
//	for (auto& blas : m_blas) {
//		VkAccelerationStructureCreateInfoKHR asCreateInfo{ VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR };
//		asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
//		asCreateInfo.flags = flags;
//		asCreateInfo.maxGeometryCount = (uint32_t)blas.asCreateGeometryInfo.size();
//		asCreateInfo.pGeometryInfos = blas.asCreateGeometryInfo.data();
//
//		// Create an acceleration structure identifier and allocate memory to
//		// store the resulting structure data
//		blas.as = m_alloc->createAcceleration(asCreateInfo);
//		m_debug.setObjectName(blas.as.accel, (std::string("Blas" + std::to_string(idx)).c_str()));
//
//		// Estimate the amount of scratch memory required to build the BLAS, and
//		// update the size of the scratch buffer that will be allocated to
//		// sequentially build all BLASes
//		VkAccelerationStructureMemoryRequirementsInfoKHR memoryRequirementsInfo{
//			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_INFO_KHR
//		};
//		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_BUILD_SCRATCH_KHR;
//		memoryRequirementsInfo.accelerationStructure = blas.as.accel;
//		memoryRequirementsInfo.buildType = VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR;
//
//		VkMemoryRequirements2 reqMem{ VK_STRUCTURE_TYPE_MEMORY_REQUIREMENTS_2 };
//		vkGetAccelerationStructureMemoryRequirementsKHR(m_device, &memoryRequirementsInfo, &reqMem);
//		VkDeviceSize scratchSize = reqMem.memoryRequirements.size;
//
//
//		blas.flags = flags;
//		maxScratch = std::max(maxScratch, scratchSize);
//
//		// Original size
//		memoryRequirementsInfo.type = VK_ACCELERATION_STRUCTURE_MEMORY_REQUIREMENTS_TYPE_OBJECT_KHR;
//		vkGetAccelerationStructureMemoryRequirementsKHR(m_device, &memoryRequirementsInfo, &reqMem);
//		originalSizes[idx] = reqMem.memoryRequirements.size;
//
//		idx++;
//	}
//
//	// Allocate the scratch buffers holding the temporary data of the
//	// acceleration structure builder
//
//	nvvk::Buffer scratchBuffer = m_alloc->createBuffer(
//		maxScratch, VK_BUFFER_USAGE_RAY_TRACING_BIT_KHR | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT);
//	VkBufferDeviceAddressInfo bufferInfo{ VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO };
//	bufferInfo.buffer = scratchBuffer.buffer;
//	VkDeviceAddress scratchAddress = vkGetBufferDeviceAddress(m_device, &bufferInfo);
//
//
//	// Query size of compact BLAS
//	VkQueryPoolCreateInfo qpci{ VK_STRUCTURE_TYPE_QUERY_POOL_CREATE_INFO };
//	qpci.queryCount = (uint32_t)m_blas.size();
//	qpci.queryType = VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR;
//	VkQueryPool queryPool;
//	vkCreateQueryPool(m_device, &qpci, nullptr, &queryPool);
//
//
//	// Create a command buffer containing all the BLAS builds
//	nvvk::CommandPool genCmdBuf(m_device, m_queueIndex);
//	int ctr{ 0 };
//	std::vector<VkCommandBuffer> allCmdBufs;
//	allCmdBufs.reserve(m_blas.size());
//	for (auto& blas : m_blas) {
//		VkCommandBuffer cmdBuf = genCmdBuf.createCommandBuffer();
//		allCmdBufs.push_back(cmdBuf);
//
//		const VkAccelerationStructureGeometryKHR* pGeometry = blas.asGeometry.data();
//		VkAccelerationStructureBuildGeometryInfoKHR bottomASInfo{
//			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR
//		};
//		bottomASInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
//		bottomASInfo.flags = flags;
//		bottomASInfo.update = VK_FALSE;
//		bottomASInfo.srcAccelerationStructure = VK_NULL_HANDLE;
//		bottomASInfo.dstAccelerationStructure = blas.as.accel;
//		bottomASInfo.geometryArrayOfPointers = VK_FALSE;
//		bottomASInfo.geometryCount = (uint32_t)blas.asGeometry.size();
//		bottomASInfo.ppGeometries = &pGeometry;
//		bottomASInfo.scratchData.deviceAddress = scratchAddress;
//
//		// Pointers of offset
//		std::vector<const VkAccelerationStructureBuildOffsetInfoKHR*> pBuildOffset(blas.asBuildOffsetInfo.size());
//		for (size_t i = 0; i < blas.asBuildOffsetInfo.size(); i++)
//			pBuildOffset[i] = &blas.asBuildOffsetInfo[i];
//
//		// Building the AS
//		vkCmdBuildAccelerationStructureKHR(cmdBuf, 1, &bottomASInfo, pBuildOffset.data());
//
//		// Since the scratch buffer is reused across builds, we need a barrier to ensure one build
//		// is finished before starting the next one
//		VkMemoryBarrier barrier{ VK_STRUCTURE_TYPE_MEMORY_BARRIER };
//		barrier.srcAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_WRITE_BIT_KHR;
//		barrier.dstAccessMask = VK_ACCESS_ACCELERATION_STRUCTURE_READ_BIT_KHR;
//		vkCmdPipelineBarrier(cmdBuf, VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR,
//			VK_PIPELINE_STAGE_ACCELERATION_STRUCTURE_BUILD_BIT_KHR, 0, 1, &barrier, 0, nullptr, 0, nullptr);
//
//		// Query the compact size
//		if (doCompaction) {
//			vkCmdWriteAccelerationStructuresPropertiesKHR(
//				cmdBuf, 1, &blas.as.accel, VK_QUERY_TYPE_ACCELERATION_STRUCTURE_COMPACTED_SIZE_KHR, queryPool, ctr++);
//		}
//	}
//	genCmdBuf.submitAndWait(allCmdBufs);
//	allCmdBufs.clear();
//
//	// Compacting all BLAS
//	if (doCompaction) {
//		VkCommandBuffer cmdBuf = genCmdBuf.createCommandBuffer();
//
//		// Get the size result back
//		std::vector<VkDeviceSize> compactSizes(m_blas.size());
//		vkGetQueryPoolResults(m_device, queryPool, 0, (uint32_t)compactSizes.size(),
//			compactSizes.size() * sizeof(VkDeviceSize), compactSizes.data(), sizeof(VkDeviceSize),
//			VK_QUERY_RESULT_WAIT_BIT);
//
//
//		// Compacting
//		std::vector<nvvk::AccelKHR> cleanupAS(m_blas.size());
//		uint32_t totOriginalSize{ 0 }, totCompactSize{ 0 };
//		for (int i = 0; i < m_blas.size(); i++) {
//			// LOGI("Reducing %i, from %d to %d \n", i, originalSizes[i], compactSizes[i]);
//			totOriginalSize += (uint32_t)originalSizes[i];
//			totCompactSize += (uint32_t)compactSizes[i];
//
//			// Creating a compact version of the AS
//			VkAccelerationStructureCreateInfoKHR asCreateInfo{
//				VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR
//			};
//			asCreateInfo.compactedSize = compactSizes[i];
//			asCreateInfo.type = VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR;
//			asCreateInfo.flags = flags;
//			auto as = m_alloc->createAcceleration(asCreateInfo);
//
//			// Copy the original BLAS to a compact version
//			VkCopyAccelerationStructureInfoKHR copyInfo{ VK_STRUCTURE_TYPE_COPY_ACCELERATION_STRUCTURE_INFO_KHR };
//			copyInfo.src = m_blas[i].as.accel;
//			copyInfo.dst = as.accel;
//			copyInfo.mode = VK_COPY_ACCELERATION_STRUCTURE_MODE_COMPACT_KHR;
//			vkCmdCopyAccelerationStructureKHR(cmdBuf, &copyInfo);
//			cleanupAS[i] = m_blas[i].as;
//			m_blas[i].as = as;
//		}
//		genCmdBuf.submitAndWait(cmdBuf);
//
//		// Destroying the previous version
//		for (auto as : cleanupAS)
//			m_alloc->destroy(as);
//
//		LOGI("------------------\n");
//		LOGI("Reducing from: %u to: %u = %u (%2.2f%s smaller) \n", totOriginalSize, totCompactSize,
//			totOriginalSize - totCompactSize, (totOriginalSize - totCompactSize) / float(totOriginalSize) * 100.f,
//			"%%");
//	}
//
//	vkDestroyQueryPool(m_device, queryPool, nullptr);
//	m_alloc->destroy(scratchBuffer);
//	m_alloc->finalizeAndReleaseStaging();
//}


// PERF: based on asset update info should update only mats, accel struct, etc
void GpuMesh::Update(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();

	geometryGroups.clear();

	for (int32 i = 0; const auto& gg : data->geometrySlots) {
		GpuGeometryGroup vgg;
		vgg.material = GpuAssetManager->GetGpuHandle(data->materials[i]);

		vk::DeviceSize vertexBufferSize = sizeof(gg.vertices[0]) * gg.vertices.size();
		vk::DeviceSize indexBufferSize = sizeof(gg.indices[0]) * gg.indices.size();

		RBuffer vertexStagingbuffer{ vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		// copy data to buffer
		vertexStagingbuffer.UploadData(gg.vertices.data(), vertexBufferSize);

		RBuffer indexStagingbuffer{ indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		// copy data to buffer
		indexStagingbuffer.UploadData(gg.indices.data(), indexBufferSize);

		// device local
		vgg.vertexBuffer.reset(new RBuffer(vertexBufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer
				| vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress));

		vgg.indexBuffer.reset(new RBuffer(indexBufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer
				| vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress));


		vgg.vertexBuffer->CopyBuffer(vertexStagingbuffer);
		vgg.indexBuffer->CopyBuffer(indexStagingbuffer);

		vgg.indexCount = static_cast<uint32>(gg.indices.size());
		vgg.vertexCount = static_cast<uint32>(gg.vertices.size());

		geometryGroups.emplace_back(std::move(vgg));
		++i;
	}

	// LOG_REPORT("{}", AssetHandlerManager::GetPodUri(podHandle));
	blas = std::make_unique<RBlas>(sizeof(Vertex), geometryGroups, //
		vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
}
