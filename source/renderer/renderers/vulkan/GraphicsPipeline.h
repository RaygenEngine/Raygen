#pragma once

#include "renderer/renderers/vulkan/Device.h"
#include "renderer/renderers/vulkan/Swapchain.h"
#include "renderer/renderers/vulkan/Descriptors.h"

#include <vulkan/vulkan.hpp>

namespace vlkn {
class GraphicsPipeline {

	vk::UniqueDescriptorSetLayout m_descriptorSetLayout;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;


public:
	GraphicsPipeline(Device* device, Swapchain* swapchain);

	vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout.get(); }

	vk::Pipeline GetPipeline() const { return m_pipeline.get(); }
	vk::PipelineLayout GetPipelineLayout() const { return m_pipelineLayout.get(); }
};
} // namespace vlkn
