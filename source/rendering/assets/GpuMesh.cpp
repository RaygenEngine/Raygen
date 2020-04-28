#include "pch.h"
#include "GpuMesh.h"

#include "rendering/assets/GpuAssetManager.h"
#include "rendering/Renderer.h"
#include "rendering/Device.h"

using namespace vl;

// PERF:
Mesh::Gpu::Gpu(PodHandle<Mesh> podHandle)
	: GpuAssetTemplate(podHandle)
{
	auto data = podHandle.Lock();

	// PERF:
	for (const auto& gg : data->geometryGroups) {

		GpuGeometryGroup vgg;

		vk::DeviceSize vertexBufferSize = sizeof(gg.vertices[0]) * gg.vertices.size();
		vk::DeviceSize indexBufferSize = sizeof(gg.indices[0]) * gg.indices.size();

		RawBuffer vertexStagingBuffer{ vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		// copy data to buffer
		vertexStagingBuffer.UploadData(gg.vertices.data(), vertexBufferSize);

		RawBuffer indexStagingBuffer{ indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		// copy data to buffer
		indexStagingBuffer.UploadData(gg.indices.data(), indexBufferSize);

		// device local
		vgg.vertexBuffer.reset(new RawBuffer(vertexBufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal));

		vgg.indexBuffer.reset(new RawBuffer(indexBufferSize,
			vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
			vk::MemoryPropertyFlagBits::eDeviceLocal));

		// copy from host to device local
		vgg.vertexBuffer->CopyBuffer(vertexStagingBuffer);
		vgg.indexBuffer->CopyBuffer(indexStagingBuffer);

		vgg.indexCount = static_cast<uint32>(gg.indices.size());

		// TODO: Convert to material index for material slot editing
		vgg.material = GpuAssetManager->GetGpuHandle(data->materials[gg.materialIndex]);

		geometryGroups.emplace_back(std::move(vgg));
	}
}
