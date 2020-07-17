#include "pch.h"
#include "SceneGeometry.h"

#include "rendering/wrappers/RBuffer.h"
#include "rendering/Device.h"

void SceneAnimatedGeometry::UploadSsbo(uint32 curFrame)
{
	if (jointMatrices.size() > 0) {
		buffers[curFrame]->UploadData(jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4));
	}
}

void SceneAnimatedGeometry::ResizeJoints(uint32 curFrame)
{
	auto i = curFrame;

	buffers[i] = std::make_unique<vl::RBuffer>(GetBufferSize(), vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

	vk::DescriptorBufferInfo bufferInfo{};

	bufferInfo
		.setBuffer(*buffers[i]) //
		.setOffset(0u)
		.setRange(GetBufferSize());
	vk::WriteDescriptorSet descriptorWrite{};

	descriptorWrite
		.setDstSet(descSets[i]) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eStorageBuffer)
		.setDescriptorCount(1u)
		.setPBufferInfo(&bufferInfo)
		.setPImageInfo(nullptr)
		.setPTexelBufferView(nullptr);

	vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
}

size_t SceneAnimatedGeometry::GetBufferSize() const
{
	return glm::max(jointMatrices.size() * sizeof(glm::mat4), 8llu);
}
