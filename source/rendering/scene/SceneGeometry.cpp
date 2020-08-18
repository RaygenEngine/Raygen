#include "pch.h"
#include "SceneGeometry.h"

#include "rendering/Device.h"
#include "rendering/wrappers/Buffer.h"

void SceneAnimatedGeometry::UploadSsbo(uint32 curFrame)
{
	if (jointMatrices.size() > 0) {
		buffer[curFrame].UploadData(jointMatrices.data(), jointMatrices.size() * sizeof(glm::mat4));
	}
}

void SceneAnimatedGeometry::ResizeJoints(uint32 curFrame)
{
	auto i = curFrame;

	buffer[i] = vl::RBuffer{ GetBufferSize(), vk::BufferUsageFlagBits::eStorageBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	vk::DescriptorBufferInfo bufferInfo{};

	bufferInfo
		.setBuffer(buffer[i]) //
		.setOffset(0u)
		.setRange(GetBufferSize());
	vk::WriteDescriptorSet descriptorWrite{};

	descriptorWrite
		.setDstSet(descSet[i]) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eStorageBuffer)
		.setDescriptorCount(1u)
		.setPBufferInfo(&bufferInfo)
		.setPImageInfo(nullptr)
		.setPTexelBufferView(nullptr);

	vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
}

void SceneAnimatedGeometry::MaybeResizeJoints(size_t newSize)
{
	if (newSize != jointMatrices.size()) {
		isDirtyResize = true;
		jointMatrices.resize(newSize);
	}
}

size_t SceneAnimatedGeometry::GetBufferSize() const
{
	return glm::max(jointMatrices.size() * sizeof(glm::mat4), 8llu);
}
