#pragma once

#include "renderer/renderers/vulkan/Device.h"
#include "renderer/renderers/vulkan/Swapchain.h"
#include "renderer/renderers/vulkan/Descriptors.h"

#include <vulkan/vulkan.hpp>

namespace vulkan {
class GraphicsPipeline {

	vk::DescriptorSetLayout m_descriptorSetLayout;

	vk::Pipeline m_pipeline;
	vk::PipelineLayout m_pipelineLayout;


public:
	GraphicsPipeline(Device* device, Swapchain* swapchain);

	vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout; }

	vk::Pipeline GetPipeline() const { return m_pipeline; }
	vk::PipelineLayout GetPipelineLayout() const { return m_pipelineLayout; }
};
} // namespace vulkan
