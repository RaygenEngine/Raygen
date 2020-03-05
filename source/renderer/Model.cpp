#include "pch/pch.h"

#include "renderer/Model.h"
#include "renderer/VulkanLayer.h"
#include "asset/AssetManager.h"


// PERF:
Model::Model(PodHandle<ModelPod> podHandle)
{
	auto& device = VulkanLayer::device;

	auto data = podHandle.Lock();

	// PERF:
	for (const auto& mesh : data->meshes) {
		for (const auto& gg : mesh.geometryGroups) {

			GPUGeometryGroup vgg;

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

			// albedo texture

			// TODO: GPU ASSETS caching
			vgg.albedoText = std::make_unique<Texture>(data->materials[gg.materialIndex].Lock()->baseColorTexture);

			// descriptors
			vgg.descriptorSet = VulkanLayer::GetModelDescriptorSet();

			// uniform sets
			vk::DescriptorBufferInfo bufferInfo{};
			bufferInfo.setBuffer(VulkanLayer::uniformBuffers.get()).setOffset(0).setRange(sizeof(UniformBufferObject));

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite.setDstSet(vgg.descriptorSet)
				.setDstBinding(0)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1u)
				.setPBufferInfo(&bufferInfo)
				.setPImageInfo(nullptr)
				.setPTexelBufferView(nullptr);

			device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);


			// images (material)

			vk::DescriptorImageInfo imageInfo{};
			imageInfo.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
				.setImageView(vgg.albedoText->view.get())
				.setSampler(vgg.albedoText->sampler.get());

			descriptorWrite.setDstSet(vgg.descriptorSet)
				.setDstBinding(1)
				.setDstArrayElement(0)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1u)
				.setPBufferInfo(nullptr)
				.setPImageInfo(&imageInfo)
				.setPTexelBufferView(nullptr);

			device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);

			geometryGroups.emplace_back(std::move(vgg));
		}
	}
}
