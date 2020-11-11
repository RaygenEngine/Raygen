#include "SceneStructs.h"

#include "rendering/Device.h"

SceneStruct::SceneStruct(size_t uboSize)
{
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		uboDescSet[i] = vl::Layouts->singleUboDescLayout.AllocDescriptorSet();

		buffer[i] = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		vk::DescriptorBufferInfo bufferInfo{};

		bufferInfo
			.setBuffer(buffer[i].handle()) //
			.setOffset(0u)
			.setRange(uboSize);
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(uboDescSet[i]) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setBufferInfo(bufferInfo);

		vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}

void SceneStruct::UploadDataToUbo(uint32 curFrame, void* data, size_t size)
{
	buffer[curFrame].UploadData(data, size);
}
