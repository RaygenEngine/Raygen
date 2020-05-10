#pragma once
#include "rendering/objects/RBuffer.h"
#include "rendering/Layouts.h"
#include "rendering/Device.h"

// SceneStructs that upload a Ubo when dirty
template<typename Ubo>
struct SceneStruct {
	Ubo ubo;

	std::array<vk::DescriptorSet, 3> descSets;
	std::array<UniquePtr<vl::RUboBuffer<Ubo>>, 3> buffers;

	std::array<bool, 3> isDirty{ true, true, true };

	void UploadUbo(uint32 curFrame) { buffers[curFrame]->UploadData(ubo); }

	SceneStruct()
	{
		for (uint32 i = 0; i < 3; ++i) {
			descSets[i] = vl::Layouts->singleUboDescLayout.GetDescriptorSet();

			buffers[i] = std::make_unique<vl::RUboBuffer<Ubo>>(vk::BufferUsageFlagBits::eUniformBuffer,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

			vk::DescriptorBufferInfo bufferInfo{};

			bufferInfo
				.setBuffer(*buffers[i]) //
				.setOffset(0u)
				.setRange(sizeof(Ubo));
			vk::WriteDescriptorSet descriptorWrite{};

			descriptorWrite
				.setDstSet(descSets[i]) //
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
};
