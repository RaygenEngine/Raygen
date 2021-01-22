#include "SceneStructs.h"

#include "rendering/Device.h"

SceneStruct::SceneStruct(size_t _uboSize)
	: uboSize(_uboSize)
{
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		uboDescSet[i] = vl::Layouts->singleUboDescLayout.AllocDescriptorSet();

		buffer[i] = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		rvk::writeDescriptorBuffer(uboDescSet[i], 0u, buffer[i].handle(), uboSize);
	}
}

void SceneStruct::UploadDataToUbo(uint32 curFrame, void* data, size_t size)
{
	buffer[curFrame].UploadData(data, size);
}
