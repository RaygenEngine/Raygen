#include "pch.h"
#include "SceneStructs.h"

#include "rendering/scene/Scene.h"

SceneCamera::SceneCamera(uint32 size)
{
	for (int32 i = 0; i < size; ++i) {
		descSets.push_back(Scene->cameraDescLayout.GetDescriptorSet());
		buffers.emplace_back(std::make_unique<vl::Buffer<Ubo>>(vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
	}
}

SceneSpotlight::SceneSpotlight(uint32 size)
{
	for (int32 i = 0; i < size; ++i) {
		descSets.push_back(Scene->spotLightDescLayout.GetDescriptorSet());
		buffers.emplace_back(std::make_unique<vl::Buffer<Ubo>>(vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent));
	}
}
