#pragma once

#include <vulkan/vulkan.hpp>

namespace rvk {

vk::UniquePipeline makeGraphicsPipeline(const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
	vk::PipelineVertexInputStateCreateInfo* pVertexInputState,
	vk::PipelineInputAssemblyStateCreateInfo* pInputAssemblyState,
	vk::PipelineTessellationStateCreateInfo* pTessellationState, vk::PipelineViewportStateCreateInfo* pViewportState,
	vk::PipelineRasterizationStateCreateInfo* pRasterizationState,
	vk::PipelineMultisampleStateCreateInfo* pMultisampleState,
	vk::PipelineDepthStencilStateCreateInfo* pDepthStencilState,
	vk::PipelineColorBlendStateCreateInfo* pColorBlendState, vk::PipelineDynamicStateCreateInfo* pDynamicState,
	vk::PipelineLayout pipeLayout, vk::RenderPass renderPass, uint32 subpassIndex = 0u,
	vk::PipelineCreateFlags flags = {}, vk::Pipeline basePipeline = {}, uint32 basePipelineIndex = -1);


vk::UniquePipeline makeComputePipeline(
	const vk::PipelineShaderStageCreateInfo& shaderStage, vk::PipelineLayout pipeLayout);

// vk::UniquePipeline makeFullscreenPipeline(vk::PipelineShaderStageCreateInfo fragmentShader,
// vk::PipelineColorBlendStateCreateInfo colorBlending, vk::PipelineLayout pipeLayout, vk::RenderPass renderPass,
// uint32 subpassIndex = 0);

vk::UniquePipelineLayout makePipelineLayoutNoPC(std::vector<vk::DescriptorSetLayout> layouts);

// Mostly intended as a utility function for rvk itself, but this can be useful if one uses dynamic PushConstant size
vk::UniquePipelineLayout makePipelineLayoutEx(
	std::initializer_list<vk::DescriptorSetLayout> layoutsIn, vk::ShaderStageFlags pcStageFlags, uint32 pcStructSize);

// Note that pcStageFlags default value is frag | vert.
// When pushing constants all flags must be exactly the same. You should specify explicitly
template<typename PushConstantStruct>
vk::UniquePipelineLayout makePipelineLayout(std::initializer_list<vk::DescriptorSetLayout> layoutsIn,
	vk::ShaderStageFlags pcStageFlags = vk::ShaderStageFlagBits::eFragment | vk::ShaderStageFlagBits::eVertex)
{
	return makePipelineLayoutEx(layoutsIn, pcStageFlags, sizeof(PushConstantStruct));
}
} // namespace rvk
