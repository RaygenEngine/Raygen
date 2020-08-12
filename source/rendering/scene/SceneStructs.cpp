#include "pch.h"
#include "SceneStructs.h"

#include "rendering/wrappers/RBuffer.h"
#include "rendering/Device.h"

SceneStruct::SceneStruct(size_t uboSize)
{
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		descSet[i] = vl::Layouts->singleUboDescLayout.GetDescriptorSet();

		buffer[i] = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		vk::DescriptorBufferInfo bufferInfo{};

		bufferInfo
			.setBuffer(buffer[i]) //
			.setOffset(0u)
			.setRange(uboSize);
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(descSet[i]) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1u)
			.setPBufferInfo(&bufferInfo)
			.setPImageInfo(nullptr)
			.setPTexelBufferView(nullptr);

		vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}

void SceneStruct::UploadDataToUbo(uint32 curFrame, void* data, size_t size)
{
	buffer[curFrame].UploadData(data, size);
}
