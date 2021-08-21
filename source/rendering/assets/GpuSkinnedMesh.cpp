#include "GpuSkinnedMesh.h"

#include "assets/pods/SkinnedMesh.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMaterialInstance.h"

using namespace vl;

GpuSkinnedMesh::GpuSkinnedMesh(PodHandle<SkinnedMesh> podHandle)
	: GpuAssetTemplate(podHandle)
{
	UpdateGeometry({});
	Update({});
}

void GpuSkinnedMesh::Update(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();

	for (int32 i = 0; auto& gg : geometryGroups) {
		gg.material = GpuAssetManager->GetGpuHandle(data->materials[i]);
		++i;
	}
}

void vl::GpuSkinnedMesh::UpdateGeometry(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();

	geometryGroups.clear();
	if (data->skinnedGeometrySlots.empty()) {
		return;
	}

	vk::DeviceSize totalVertexBufferSize = 0;
	vk::DeviceSize totalIndexBufferSize = 0;

	vk::DeviceSize stagingVertexSize = 0;
	vk::DeviceSize stagingIndexSize = 0;

	for (auto& gg : data->skinnedGeometrySlots) {
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


	RBuffer vertexStagingbuffer{ stagingVertexSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
	RBuffer indexStagingbuffer{ stagingIndexSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	uint32 indexBufferOffset = 0;
	uint32 vertexBufferOffset = 0;
	uint32 indexOffset = 0;

	for (int32 i = 0; const auto& gg : data->skinnedGeometrySlots) {
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
		indexOffset += vgg.indexCount;

		vertexBufferOffset = static_cast<uint32>(
			combinedVertexBuffer.CopyBufferAt(vertexStagingbuffer, vertexBufferOffset, vertexBufferSize));
		indexBufferOffset = static_cast<uint32>(
			combinedIndexBuffer.CopyBufferAt(indexStagingbuffer, indexBufferOffset, indexBufferSize));

		geometryGroups.emplace_back(std::move(vgg));
		++i;
	}

	// TODO: Skinned mesh blas
	// vgg.blas = BottomLevelAs(sizeof(SkinnedVertex), vgg,
	// vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
}
