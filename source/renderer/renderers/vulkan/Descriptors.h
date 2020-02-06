#pragma once

#include "vulkan/vulkan.hpp"


namespace vlkn {

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
	std::vector<vk::UniqueBuffer> m_uniformBuffers;
	std::vector<vk::UniqueDeviceMemory> m_uniformBuffersMemory;

	vk::UniqueDescriptorPool m_descriptorPool;
	std::vector<vk::UniqueDescriptorSet> m_descriptorSets;

public:
	Descriptors(Device* device, Swapchain* swapchain, GraphicsPipeline* graphicsPipeline);

	std::vector<vk::Buffer> GetUniformBuffers() const { return vk::uniqueToRaw(m_uniformBuffers); }
	std::vector<vk::DeviceMemory> GetUniformBuffersMemory() const { return vk::uniqueToRaw(m_uniformBuffersMemory); }
	std::vector<vk::DescriptorSet> GetDescriptorSets() const { return vk::uniqueToRaw(m_descriptorSets); }
};
} // namespace vlkn
