#include "pch.h"
#include "GpuMesh.h"

#include "assets/pods/Mesh.h"
#include "rendering/assets/GpuAssetBase.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/wrappers/BottomLevelAs.h"
#include "rendering/wrappers/Buffer.h"
#include "assets/AssetRegistry.h"
#include "rendering/Instance.h"

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
		vgg.vertexBuffer = RBuffer{ vertexBufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer
				| vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };

		vgg.indexBuffer = RBuffer{ indexBufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer
				| vk::BufferUsageFlagBits::eShaderDeviceAddress,
			vk::MemoryPropertyFlagBits::eDeviceLocal, vk::MemoryAllocateFlagBits::eDeviceAddress };


		vgg.vertexBuffer.CopyBuffer(vertexStagingbuffer);
		vgg.indexBuffer.CopyBuffer(indexStagingbuffer);

		vgg.indexCount = static_cast<uint32>(gg.indices.size());
		vgg.vertexCount = static_cast<uint32>(gg.vertices.size());

		geometryGroups.emplace_back(std::move(vgg));
		++i;
	}

	if (geometryGroups.size() > 0) {
		blas = BottomLevelAs(sizeof(Vertex), geometryGroups, //
			vk::BuildAccelerationStructureFlagBitsKHR::ePreferFastTrace);
	}
	else {
		blas.handle.reset();
	}
}
