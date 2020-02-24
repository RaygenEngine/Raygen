#include "pch/pch.h"
#include "Descriptors.h"


#include "renderer/DeviceWrapper.h"
#include "renderer/Swapchain.h"
#include "renderer/GraphicsPipeline.h"
#include "renderer/Texture.h"


#define POOL_PER_IMAGE_SET_COUNT 100

vk::UniqueDescriptorPool CreateDescriptorPool(DeviceWrapper& device, uint32 setCount)
{
	std::array<vk::DescriptorPoolSize, 2> poolSizes{};
	// for global uniforms
	poolSizes[0].setType(vk::DescriptorType::eUniformBuffer).setDescriptorCount(setCount);
	// for image sampler combinations
	poolSizes[1].setType(vk::DescriptorType::eCombinedImageSampler).setDescriptorCount(setCount);

	vk::DescriptorPoolCreateInfo poolInfo{};
	poolInfo.setPoolSizeCount(static_cast<uint32_t>(poolSizes.size()))
		.setPPoolSizes(poolSizes.data())
		.setMaxSets(POOL_PER_IMAGE_SET_COUNT * setCount);

	return device->createDescriptorPoolUnique(poolInfo);
}

// WIP: descriptor set should describe
// 1. material layout (samplers etc)
// 2. other uniforms
Descriptors::Descriptors(DeviceWrapper& device, Swapchain* swapchain, GraphicsPipeline* graphicsPipeline)
	: m_assocDevice(device)
	, m_assocSwapchain(swapchain)
	, m_assocGraphicsPipeline(graphicsPipeline)
{
	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	m_uniformBuffers.resize(m_assocSwapchain->GetImageCount());
	m_uniformBuffersMemory.resize(m_assocSwapchain->GetImageCount());

	for (size_t i = 0; i < m_assocSwapchain->GetImageCount(); i++) {
		device.CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, m_uniformBuffers[i],
			m_uniformBuffersMemory[i]);
	}

	m_availableSetCount = POOL_PER_IMAGE_SET_COUNT * m_assocSwapchain->GetImageCount();

	m_descriptorPools.emplace_back(CreateDescriptorPool(m_assocDevice, m_assocSwapchain->GetImageCount()));
}
std::vector<vk::DescriptorSet> Descriptors::CreateDescriptorSets()
{
	// TODO: This system should check if each active pool for allocation space, (also free pool if empty?)
	if (m_availableSetCount < POOL_PER_IMAGE_SET_COUNT * m_assocSwapchain->GetImageCount()) {
		LOG_WARN("Requested descriptor set count is bigger than the available set count of current descriptor pool");
		m_descriptorPools.emplace_back(CreateDescriptorPool(m_assocDevice, m_assocSwapchain->GetImageCount()));
		m_availableSetCount = POOL_PER_IMAGE_SET_COUNT * m_assocSwapchain->GetImageCount();
	}

	std::vector<vk::DescriptorSetLayout> layouts(
		m_assocSwapchain->GetImageCount(), m_assocGraphicsPipeline->GetDescriptorSetLayout());
	vk::DescriptorSetAllocateInfo allocInfo{};

	allocInfo.setDescriptorPool(m_descriptorPools.back().get())
		.setDescriptorSetCount(m_assocSwapchain->GetImageCount())
		.setPSetLayouts(layouts.data());

	m_availableSetCount -= m_assocSwapchain->GetImageCount();

	// WIP: are those destructed?
	return m_assocDevice->allocateDescriptorSets(allocInfo);
}
