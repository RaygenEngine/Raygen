#include "pch.h"
#include "GpuSkinnedMesh.h"

#include "assets/pods/SkinnedMesh.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMaterialInstance.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"

using namespace vl;

GpuSkinnedMesh::GpuSkinnedMesh(PodHandle<SkinnedMesh> podHandle)
	: GpuAssetTemplate(podHandle)
{
	auto data = podHandle.Lock();

	// PERF:
	for (int32 i = 0; const auto& gg : data->skinnedGeometrySlots) {
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
		vgg.vertexBuffer
			= RBuffer{ vertexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				  vk::MemoryPropertyFlagBits::eDeviceLocal };

		vgg.indexBuffer
			= RBuffer{ indexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				  vk::MemoryPropertyFlagBits::eDeviceLocal };

		// copy from host to device local
		vgg.vertexBuffer.CopyBuffer(vertexStagingbuffer);
		vgg.indexBuffer.CopyBuffer(indexStagingbuffer);

		vgg.indexCount = static_cast<uint32>(gg.indices.size());

		geometryGroups.emplace_back(std::move(vgg));
		++i;
	}
}

void GpuSkinnedMesh::Update(const AssetUpdateInfo& info)
{
	auto data = podHandle.Lock();

	for (int32 i = 0; auto& gg : geometryGroups) {
		gg.material = GpuAssetManager->GetGpuHandle(data->materials[i]);
		++i;
	}
}
