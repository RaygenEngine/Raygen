#include "pch/pch.h"
#include "Descriptors.h"


#include "renderer/renderers/vulkan/Device.h"
#include "renderer/renderers/vulkan/Swapchain.h"
#include "renderer/renderers/vulkan/GraphicsPipeline.h"


namespace vlkn {
Descriptors::Descriptors(Device* device, Swapchain* swapchain, GraphicsPipeline* graphicsPipeline)
{
	uint32 setCount = swapchain->GetImageCount();

	// uniform buffer (memory)

	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(setCount);
	m_uniformBuffersMemory.resize(setCount);

	for (size_t i = 0; i < setCount; i++) {
		device->CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, m_uniformBuffers[i],
			m_uniformBuffersMemory[i]);
	}

	// descriptor pool

	vk::DescriptorPoolSize poolSize{};
	poolSize.setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(static_cast<uint32>(setCount));

	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo.setPoolSizeCount(1u).setPPoolSizes(&poolSize).setMaxSets(static_cast<uint32>(setCount));

	m_descriptorPool = device->createDescriptorPoolUnique(poolInfo);

	// descriptor sets

	std::vector<vk::DescriptorSetLayout> layouts(setCount, graphicsPipeline->GetDescriptorSetLayout());
	vk::DescriptorSetAllocateInfo allocInfo{};
	allocInfo.setDescriptorPool(m_descriptorPool.get())
		.setDescriptorSetCount(static_cast<uint32>(setCount))
		.setPSetLayouts(layouts.data());

	m_descriptorSets.resize(setCount);
	m_descriptorSets = device->allocateDescriptorSetsUnique(allocInfo);

	for (uint32 i = 0; i < setCount; ++i) {
		vk::DescriptorBufferInfo bufferInfo{};
		bufferInfo.setBuffer(m_uniformBuffers[i].get()).setOffset(0).setRange(sizeof(UniformBufferObject));

		vk::WriteDescriptorSet descriptorWrite{};
		descriptorWrite.setDstSet(m_descriptorSets[i].get())
			.setDstBinding(0)
			.setDstArrayElement(0)
			.setDescriptorType(vk::DescriptorType::eUniformBuffer)
			.setDescriptorCount(1u)
			.setPBufferInfo(&bufferInfo)
			.setPImageInfo(nullptr)
			.setPTexelBufferView(nullptr);

		device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
}
} // namespace vlkn
