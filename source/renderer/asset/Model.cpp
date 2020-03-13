#include "pch.h"
#include "renderer/asset/Model.h"

#include "asset/AssetManager.h"
#include "renderer/asset/GpuAssetManager.h"
#include "renderer/VulkanLayer.h"
#include "renderer/wrapper/Device.h"

// PERF:
GpuAssetBaseTyped<ModelPod>::GpuAssetBaseTyped(PodHandle<ModelPod> podHandle)
{
	auto data = podHandle.Lock();

	// PERF:
	for (const auto& mesh : data->meshes) {
		for (const auto& gg : mesh.geometryGroups) {

			GPUGeometryGroup vgg;

			vk::DeviceSize vertexBufferSize = sizeof(gg.vertices[0]) * gg.vertices.size();
			vk::DeviceSize indexBufferSize = sizeof(gg.indices[0]) * gg.indices.size();

			Buffer vertexStagingBuffer{ vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

			// copy data to buffer
			vertexStagingBuffer.UploadData(gg.vertices.data(), vertexBufferSize);

			Buffer indexStagingBuffer{ indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

			// copy data to buffer
			indexStagingBuffer.UploadData(gg.indices.data(), indexBufferSize);

			// device local
			vgg.vertexBuffer.reset(new Buffer(vertexBufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal));

			vgg.indexBuffer.reset(new Buffer(indexBufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal));

			// copy from host to device local
			vgg.vertexBuffer->CopyBuffer(vertexStagingBuffer);
			vgg.indexBuffer->CopyBuffer(indexStagingBuffer);

			vgg.indexCount = static_cast<uint32>(gg.indices.size());

			// WIP: asset caching
			vgg.material = GpuAssetManager.GetGpuHandle(data->materials[gg.materialIndex]);

			geometryGroups.emplace_back(std::move(vgg));
		}
	}
}
