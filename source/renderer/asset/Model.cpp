#include "pch.h"
#include "renderer/asset/Model.h"

#include "asset/AssetManager.h"
#include "renderer/VulkanLayer.h"
#include "renderer/asset/GpuAssetManager.h"
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
			vgg.vertexBuffer = std::make_unique<Buffer>(vertexBufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			vgg.indexBuffer = std::make_unique<Buffer>(indexBufferSize,
				vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eIndexBuffer,
				vk::MemoryPropertyFlagBits::eDeviceLocal);

			// copy host to device local
			vgg.vertexBuffer->CopyBuffer(vertexStagingBuffer);
			vgg.indexBuffer->CopyBuffer(indexStagingBuffer);

			vgg.indexCount = static_cast<uint32>(gg.indices.size());

			// albedo texture

			// WIP: asset caching
			vgg.material = GpuAssetManager.GetGpuHandle(data->materials[gg.materialIndex]);

			// descriptors
			vgg.descriptorSet = Layer->GetModelDescriptorSet();

			// globals uniform sets
			vk::DescriptorBufferInfo bufferInfo{};
			bufferInfo
				.setBuffer(*Layer->globalsUBO) //
				.setOffset(0u)
				.setRange(sizeof(UBO_Globals));

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite
				.setDstSet(vgg.descriptorSet) //
				.setDstBinding(0u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1u)
				.setPBufferInfo(&bufferInfo)
				.setPImageInfo(nullptr)
				.setPTexelBufferView(nullptr);

			Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);

			// material uniform sets CHECK: (those buffers should be set again when material changes)
			auto& mat = GpuAssetManager.LockHandle(vgg.material);
			bufferInfo
				.setBuffer(*mat.materialUBO) //
				.setOffset(0u)
				.setRange(sizeof(UBO_Material));

			descriptorWrite
				.setDstSet(vgg.descriptorSet) //
				.setDstBinding(1u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eUniformBuffer)
				.setDescriptorCount(1u)
				.setPBufferInfo(&bufferInfo)
				.setPImageInfo(nullptr)
				.setPTexelBufferView(nullptr);

			Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);


			// images (material)

			auto UpdateImageSamplerInDescriptorSet = [&](GpuHandle<TexturePod> texture, uint32 dstBinding) {
				auto& text = GpuAssetManager.LockHandle(texture);

				vk::DescriptorImageInfo imageInfo{};
				imageInfo
					.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
					.setImageView(text.view.get())
					.setSampler(text.sampler.get());

				vk::WriteDescriptorSet descriptorWrite{};
				descriptorWrite
					.setDstSet(vgg.descriptorSet) //
					.setDstBinding(dstBinding)
					.setDstArrayElement(0u)
					.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
					.setDescriptorCount(1u)
					.setPBufferInfo(nullptr)
					.setPImageInfo(&imageInfo)
					.setPTexelBufferView(nullptr);

				Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
			};

			UpdateImageSamplerInDescriptorSet(mat.baseColorTexture, 2u);
			UpdateImageSamplerInDescriptorSet(mat.metallicRoughnessTexture, 3u);
			UpdateImageSamplerInDescriptorSet(mat.occlusionTexture, 4u);
			UpdateImageSamplerInDescriptorSet(mat.normalTexture, 5u);
			UpdateImageSamplerInDescriptorSet(mat.emissiveTexture, 6u);

			geometryGroups.emplace_back(std::move(vgg));
		}
	}
}