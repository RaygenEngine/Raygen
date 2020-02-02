#pragma once

#include "vulkan/vulkan.hpp"


namespace vulkan {

class Device;
class Swapchain;
class GraphicsPipeline;

struct UniformBufferObject {
	glm::mat4 model;
	glm::mat4 view;
	glm::mat4 proj;
};

// WIP: rename (uniform descriptors??)
class Descriptors {

	// uniforms
	std::vector<vk::Buffer> m_uniformBuffers;
	std::vector<vk::DeviceMemory> m_uniformBuffersMemory;

	vk::DescriptorPool m_descriptorPool;
	std::vector<vk::DescriptorSet> m_descriptorSets;

public:
	Descriptors(Device* device, Swapchain* swapchain, GraphicsPipeline* graphicsPipeline);

	std::vector<vk::Buffer> GetUniformBuffers() const { return m_uniformBuffers; }
	std::vector<vk::DeviceMemory> GetUniformBuffersMemory() const { return m_uniformBuffersMemory; }
	vk::DescriptorPool GetDescriptorPool() const { return m_descriptorPool; }
	std::vector<vk::DescriptorSet> GetDescriptorSets() const { return m_descriptorSets; }
};
} // namespace vulkan
