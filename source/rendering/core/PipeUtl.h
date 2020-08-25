#pragma once

namespace rvk {
vk::UniquePipeline makePostProcPipeline(const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
	vk::PipelineColorBlendStateCreateInfo colorBlending, vk::PipelineLayout pipeLayout, vk::RenderPass renderPass,
	uint32 subpassIndex = 0);


// TODO: make this
// vk::UniquePipeline makeFullscreenPipeline(vk::PipelineShaderStageCreateInfo fragmentShader,
// vk::PipelineColorBlendStateCreateInfo colorBlending, vk::PipelineLayout pipeLayout, vk::RenderPass renderPass,
// uint32 subpassIndex = 0);

vk::UniquePipelineLayout makeLayoutNoPC(std::vector<vk::DescriptorSetLayout> layouts);
} // namespace rvk
