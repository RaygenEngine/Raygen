#include "GpuMesh.h"

#include "assets/AssetRegistry.h"
#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMaterialInstance.h"

using namespace vl;

GpuMesh::GpuMesh(PodHandle<Mesh> podHandle)
	: GpuAssetTemplate(podHandle)
{
	Update({});
}


// PERF: based on asset update info should update only mats, accel struct, etc
void GpuMesh::Update(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();

	ClearDependencies();

	for (int32 i = 0; auto& gg : geometryGroups) {
		gg.material = GpuAssetManager->GetGpuHandle(data->materials[i]);
		++i;

		if (i > data->materials.size()) {
			LOG_WARN(
				"Incompatible size of GpuMesh, Gpu Geom Groups did not match Cpu groups. Fix this for runtime meshes.");
			return;
		}
	}

	// TODO: Binary info.HasFlag(BuildGeometry)
	UpdateGeometry({});
}

void GpuMesh::UpdateGeometry(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();

	geometryGroups.clear();
	if (data->geometrySlots.empty()) {
		return;
	}

	vk::DeviceSize totalVertexBufferSize = 0;
	vk::DeviceSize totalIndexBufferSize = 0;

	vk::DeviceSize stagingVertexSize = 0;
	vk::DeviceSize stagingIndexSize = 0;


	for (auto& gg : data->geometrySlots) {
		const vk::DeviceSize ggVertexSize = sizeof(gg.vertices[0]) * gg.vertices.size();
		const vk::DeviceSize ggIndexSize = sizeof(gg.indices[0]) * gg.indices.size();
		totalVertexBufferSize += ggVertexSize;
		totalIndexBufferSize += ggIndexSize;

		stagingVertexSize = std::max(stagingVertexSize, ggVertexSize);
		stagingIndexSize = std::max(stagingIndexSize, ggIndexSize);
	}

	combinedVertexBuffer = { totalVertexBufferSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer
			| vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress
			| vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	combinedIndexBuffer = { totalIndexBufferSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer
			| vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress
			| vk::BufferUsageFlagBits::eAccelerationStructureBuildInputReadOnlyKHR,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	indexOffsetBuffer = { sizeof(uint32) * data->geometrySlots.size(),
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer
			| vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	primitiveOffsetBuffer = { sizeof(uint32) * data->geometrySlots.size(),
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer
			| vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eShaderDeviceAddress,
		vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

	DEBUG_NAME(combinedVertexBuffer, "Vertex Buffer " + AssetRegistry::GetPodUri(podHandle));
	DEBUG_NAME(combinedIndexBuffer, "Index Buffer " + AssetRegistry::GetPodUri(podHandle));

	RBuffer vertexStagingbuffer{ stagingVertexSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
	RBuffer indexStagingbuffer{ stagingIndexSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
	RBuffer indexOffsetStagingbuffer{ sizeof(uint32) * data->geometrySlots.size(),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	RBuffer primitiveOffsetStagingbuffer{ sizeof(uint32) * data->geometrySlots.size(),
		vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	DEBUG_NAME(vertexStagingbuffer, "Staging Vertex Buffer " + AssetRegistry::GetPodUri(podHandle));
	DEBUG_NAME(indexStagingbuffer, "Staging Index Buffer " + AssetRegistry::GetPodUri(podHandle));

	uint32 indexBufferOffset = 0;
	uint32 vertexBufferOffset = 0;
	uint32 indexOffset = 0;

	std::vector<uint32> indexOffsets;
	std::vector<uint32> primitiveOffsets;

	for (int32 i = 0; const auto& gg : data->geometrySlots) {
		GpuGeometryGroup vgg;
		vgg.material = GpuAssetManager->GetGpuHandle(data->materials[i]);

		vk::DeviceSize vertexBufferSize = sizeof(gg.vertices[0]) * gg.vertices.size();
		vk::DeviceSize indexBufferSize = sizeof(gg.indices[0]) * gg.indices.size();

		// copy data to buffer
		vertexStagingbuffer.UploadData(gg.vertices.data(), vertexBufferSize);

		// copy data to buffer
		indexStagingbuffer.UploadData(gg.indices.data(), indexBufferSize);

		vgg.indexCount = static_cast<uint32>(gg.indices.size());
		vgg.vertexCount = static_cast<uint32>(gg.vertices.size());

		vgg.vertexBufferOffset = vertexBufferOffset;
		vgg.indexBufferOffset = indexBufferOffset;

		vgg.indexOffset = indexOffset;
		vgg.primOffset = indexBufferOffset / 12;

		indexOffsets.emplace_back(indexOffset);
		primitiveOffsets.emplace_back(indexBufferOffset / 12);
		indexOffset += vgg.vertexCount;

		vertexBufferOffset = static_cast<uint32>(
			combinedVertexBuffer.CopyBufferAt(vertexStagingbuffer, vertexBufferOffset, vertexBufferSize));
		indexBufferOffset = static_cast<uint32>(
			combinedIndexBuffer.CopyBufferAt(indexStagingbuffer, indexBufferOffset, indexBufferSize));

		geometryGroups.emplace_back(std::move(vgg));
		++i;
	}

	for (auto& vgg : geometryGroups) {
		AddDependency(vgg.material);

		vgg.blas = BottomLevelAs(sizeof(Vertex), combinedVertexBuffer, combinedIndexBuffer, vgg,
			vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
	}

	indexOffsetStagingbuffer.UploadData(indexOffsets.data(), indexOffsets.size() * sizeof(uint32));
	indexOffsetBuffer.CopyBuffer(indexOffsetStagingbuffer);


	primitiveOffsetStagingbuffer.UploadData(primitiveOffsets.data(), primitiveOffsets.size() * sizeof(uint32));
	primitiveOffsetBuffer.CopyBuffer(primitiveOffsetStagingbuffer);
}
