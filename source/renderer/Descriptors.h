#pragma once

#include "vulkan/vulkan.hpp"


class DeviceWrapper;
class Swapchain;
class GraphicsPipeline;
class Texture;

struct UniformBufferObject {
	glm::mat4 view;
	glm::mat4 proj;
};


// WIP: rename (uniform descriptors??)
class Descriptors {

	std::vector<vk::UniqueDescriptorPool> m_descriptorPools;

	// global uniforms
	std::vector<vk::UniqueBuffer> m_uniformBuffers;
	std::vector<vk::UniqueDeviceMemory> m_uniformBuffersMemory;

	uint32 m_availableSetCount;

	DeviceWrapper& m_assocDevice;
	Swapchain* m_assocSwapchain;
	GraphicsPipeline* m_assocGraphicsPipeline;


public:
	Descriptors(DeviceWrapper& device, Swapchain* swapchain, GraphicsPipeline* graphicsPipeline);

	std::vector<vk::DescriptorSet> CreateDescriptorSets();
	vk::DescriptorPool GetDescriptorPool() { return m_descriptorPools.back().get(); }
	std::vector<vk::Buffer> GetUniformBuffers() { return vk::uniqueToRaw(m_uniformBuffers); }
	std::vector<vk::DeviceMemory> GetUniformBuffersMemory() { return vk::uniqueToRaw(m_uniformBuffersMemory); }
};
