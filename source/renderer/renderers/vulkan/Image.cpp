#include "pch/pch.h"

#include "renderer/renderers/vulkan/Model.h"
#include "asset/AssetManager.h"


namespace vlkn {

// PERF:
Model::Model(Device* device, PodHandle<ModelPod> handle)
{
	auto data = handle.Lock();

	// PERF:
	for (const auto& mesh : data->meshes) {
		for (const auto& gg : mesh.geometryGroups) {

			GeometryGroup vgg;

			vk::DeviceSize vertexBufferSize = sizeof(gg.vertices[0]) * gg.vertices.size();
			vk::DeviceSize indexBufferSize = sizeof(gg.indices[0]) * gg.indices.size();

			vk::UniqueBuffer vertexStagingBuffer;
			vk::UniqueDeviceMemory vertexStagingBufferMemory;
			device->CreateBuffer(vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				vertexStagingBuffer, vertexStagingBufferMemory);

			vk::UniqueBuffer indexStagingBuffer;
			vk::UniqueDeviceMemory indexStagingBufferMemory;
			device->CreateBuffer(indexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent,
				indexStagingBuffer, indexStagingBufferMemory);

			byte* vertexDstData
				= static_cast<byte*>(device->mapMemory(vertexStagingBufferMemory.get(), 0, vertexBufferSize));
			byte* indexDstData
				= static_cast<byte*>(device->mapMemory(indexStagingBufferMemory.get(), 0, indexBufferSize));

			memcpy(vertexDstData, gg.vertices.data(), vertexBufferSize);
			memcpy(indexDstData, gg.indices.data(), indexBufferSize);

			device->unmapMemory(vertexStagingBufferMemory.get());
			device->unmapMemory(indexStagingBufferMemory.get());

			// device local
			device->CreateBuffer(vertexBufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vgg.vertexBuffer, vgg.vertexBufferMemory);

			device->CreateBuffer(indexBufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal, vgg.indexBuffer, vgg.indexBufferMemory);

			// copy host to dlocal
			device->CopyBuffer(vertexStagingBuffer.get(), vgg.vertexBuffer.get(), vertexBufferSize);

			device->CopyBuffer(indexStagingBuffer.get(), vgg.indexBuffer.get(), indexBufferSize);

			vgg.indexCount = static_cast<uint32>(gg.indices.size());

			// TODO: check moves
			m_geometryGroups.emplace_back(std::move(vgg));
		}
	}
}
} // namespace vlkn
