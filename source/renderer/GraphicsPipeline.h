#pragma once

#include "renderer/DeviceWrapper.h"
#include "renderer/Swapchain.h"
#include "renderer/Descriptors.h"

#include <vulkan/vulkan.hpp>

class GraphicsPipeline {

	vk::UniqueDescriptorSetLayout m_descriptorSetLayout;

	vk::UniquePipeline m_pipeline;
	vk::UniquePipelineLayout m_pipelineLayout;


public:
	GraphicsPipeline(DeviceWrapper& device, Swapchain* swapchain);

	vk::DescriptorSetLayout GetDescriptorSetLayout() const { return m_descriptorSetLayout.get(); }

	vk::Pipeline GetPipeline() const { return m_pipeline.get(); }
	vk::PipelineLayout GetPipelineLayout() const { return m_pipelineLayout.get(); }
};
