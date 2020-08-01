#include "pch.h"
#include "GpuMesh.h"

#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/wrappers/RBuffer.h"

using namespace vl;

GpuMesh::GpuMesh(PodHandle<Mesh> podHandle)
	: GpuAssetTemplate(podHandle)
{
	auto data = podHandle.Lock();

	Update({});
}

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

		// copy from host to device local
		vgg.vertexBuffer->CopyBuffer(vertexStagingbuffer);
		vgg.indexBuffer->CopyBuffer(indexStagingbuffer);

		vgg.indexCount = static_cast<uint32>(gg.indices.size());
		vgg.vertexCount = static_cast<uint32>(gg.vertices.size());

		geometryGroups.emplace_back(std::move(vgg));
		++i;
	}

	std::vector<vk::AccelerationStructureCreateGeometryTypeInfoKHR> asCreateGeomInfos{};
	std::vector<vk::AccelerationStructureGeometryKHR*> asGeoms;
	std::vector<vk::AccelerationStructureBuildOffsetInfoKHR*> asBuildOffsetInfos{};
	for (const auto& vgg : geometryGroups) {
		// for acceleration structure creation
		vk::AccelerationStructureCreateGeometryTypeInfoKHR asCreateGeomInfo{};
		asCreateGeomInfo
			.setAllowsTransforms(VK_FALSE) //
			.setGeometryType(vk::GeometryTypeKHR::eTriangles)
			.setIndexType(vk::IndexType::eUint32)
			.setVertexFormat(vk::Format::eR32G32B32Sfloat)
			.setMaxPrimitiveCount(vgg.indexCount / 3u)
			.setMaxVertexCount(vgg.vertexCount);

		asCreateGeomInfos.emplace_back(asCreateGeomInfo);

		vk::DeviceOrHostAddressConstKHR dohacIndex{};
		dohacIndex.setDeviceAddress(vgg.indexBuffer->GetAddress()); // device local

		vk::DeviceOrHostAddressConstKHR dohacVertex{};
		dohacVertex.setDeviceAddress(vgg.vertexBuffer->GetAddress()); // device local

		vk::AccelerationStructureGeometryTrianglesDataKHR asGeomTrigData{};
		asGeomTrigData
			.setIndexData(dohacIndex) //
			.setIndexType(vk::IndexType::eUint32)
			.setVertexData(dohacVertex)
			.setVertexFormat(vk::Format::eR32G32B32Sfloat)
			.setVertexStride(offsetof(Vertex, position));
		//.setTransformData(); NEXT: should be identity or setAllowsTransforms = false?

		vk::AccelerationStructureGeometryDataKHR asGeomData{};
		asGeomData.setTriangles(asGeomTrigData);

		asGeoms.push_back(new vk::AccelerationStructureGeometryKHR{ vk::GeometryTypeKHR::eTriangles, asGeomData });
		asBuildOffsetInfos.push_back(new vk::AccelerationStructureBuildOffsetInfoKHR{ vgg.indexCount / 3u });
	}

	vk::AccelerationStructureCreateInfoKHR asCreateInfo{};
	asCreateInfo
		.setMaxGeometryCount(static_cast<uint32>(data->geometrySlots.size())) //
		.setPGeometryInfos(asCreateGeomInfos.data())
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel);

	accelStruct = Device->createAccelerationStructureKHRUnique(asCreateInfo);


	// Acceleration structure build requires some scratch space to store temporary information
	vk::AccelerationStructureMemoryRequirementsInfoKHR asMemReqsInfo{};
	asMemReqsInfo
		.setBuildType(vk::AccelerationStructureBuildTypeKHR::eDevice) //
		.setAccelerationStructure(accelStruct.get());

	auto memReqs = Device->getAccelerationStructureMemoryRequirementsKHR(asMemReqsInfo);

	const vk::DeviceSize scratchBufferSize = memReqs.memoryRequirements.size;
	LOG_REPORT("Size reqs {}", scratchBufferSize / 1000000.0);

	RBuffer scratchBuffer{ scratchBufferSize,
		vk::BufferUsageFlagBits::eRayTracingKHR | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	vk::DeviceOrHostAddressKHR dohacScratch{};
	dohacScratch.setDeviceAddress(scratchBuffer.GetAddress()); // device local

	vk::AccelerationStructureBuildGeometryInfoKHR asBuildGeomInfo{};
	asBuildGeomInfo
		.setDstAccelerationStructure(accelStruct.get()) //
		.setSrcAccelerationStructure(nullptr)           // build from scratch for now
		.setUpdate(VK_FALSE)
		.setScratchData(dohacScratch)
		.setType(vk::AccelerationStructureTypeKHR::eBottomLevel)
		.setGeometryCount(static_cast<uint32>(data->geometrySlots.size()))
		.setGeometryArrayOfPointers(VK_TRUE) // CHECK:
		.setPpGeometries(asGeoms.data());


	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlagBits::eOneTimeSubmit);

	Device->mainCmdBuffer.begin(beginInfo);


	Device->mainCmdBuffer.buildAccelerationStructureKHR(1u, &asBuildGeomInfo, asBuildOffsetInfos.data());
	Device->mainCmdBuffer.end();


	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u);
	submitInfo.setPCommandBuffers(&Device->mainCmdBuffer);

	Device->mainQueue.submit(1u, &submitInfo, {});
	Device->mainQueue.waitIdle();


	// clear
	for (size_t i = 0; i < (data->geometrySlots.size()); ++i) {
		delete asGeoms[i];
		delete asBuildOffsetInfos[i];
	}
}
