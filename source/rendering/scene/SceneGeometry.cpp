#include "SceneGeometry.h"

#include "rendering/Device.h"

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

	rvk::writeDescriptorBuffer(descSet[i], 0u, buffer[i].handle(), GetBufferSize(), vk::DescriptorType::eStorageBuffer);
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
