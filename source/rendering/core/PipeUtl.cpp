#include "pch.h"
#include "PipeUtl.h"

#include "rendering/Device.h"

namespace rvk {
vk::UniquePipeline makePostProcPipeline(const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
	vk::PipelineLayout pipeLayout, vk::RenderPass renderPass,
	std::optional<vk::PipelineColorBlendStateCreateInfo> colorBlending, uint32 subpassIndex)
{

	// Keep this outside the if below. Pointer required until the final device call.
	vk::PipelineColorBlendAttachmentState defaultBlendAttachment{};
	defaultBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
		.setBlendEnable(VK_FALSE);


	if (!colorBlending.has_value()) {
		vk::PipelineColorBlendStateCreateInfo defColorBlend{};
		defColorBlend
			.setLogicOpEnable(VK_FALSE) //
			.setLogicOp(vk::LogicOp::eCopy)
			.setAttachments(defaultBlendAttachment)
			.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

		colorBlending = defColorBlend;
	}

	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly
		.setTopology(vk::PrimitiveTopology::eTriangleList) //
		.setPrimitiveRestartEnable(VK_FALSE);

	// those are dynamic so they will be updated when needed
	vk::Viewport viewport{};
	vk::Rect2D scissor{};

	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState
		.setViewportCount(1u) //
		.setPViewports(&viewport)
		.setScissorCount(1u)
		.setPScissors(&scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer
		.setDepthClampEnable(VK_FALSE) //
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eBack)
		.setFrontFace(vk::FrontFace::eClockwise)
		.setDepthBiasEnable(VK_FALSE)
		.setDepthBiasConstantFactor(0.f)
		.setDepthBiasClamp(0.f)
		.setDepthBiasSlopeFactor(0.f);

	vk::PipelineMultisampleStateCreateInfo multisampling{};
	multisampling
		.setSampleShadingEnable(VK_FALSE) //
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setMinSampleShading(1.f)
		.setPSampleMask(nullptr)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE);

	// dynamic states
	vk::DynamicState dynamicStates[] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo
		.setDynamicStateCount(2u) //
		.setPDynamicStates(dynamicStates);

	// depth and stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil
		.setDepthTestEnable(VK_FALSE) //
		.setDepthWriteEnable(VK_FALSE)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setMinDepthBounds(0.0f) // Optional
		.setMaxDepthBounds(1.0f) // Optional
		.setStencilTestEnable(VK_FALSE)
		.setFront({}) // Optional
		.setBack({}); // Optional

	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStageCount(static_cast<uint32>(shaderStages.size())) //
		.setPStages(shaderStages.data())
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending.value())
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(pipeLayout)
		.setRenderPass(renderPass)
		.setSubpass(subpassIndex)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return vl::Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

vk::UniquePipelineLayout makeLayoutNoPC(std::vector<vk::DescriptorSetLayout> layoutsIn)
{
	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(static_cast<uint32>(layoutsIn.size())) //
		.setPSetLayouts(layoutsIn.data())
		.setPushConstantRangeCount(0u)
		.setPPushConstantRanges(nullptr);

	return vl::Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

// vk::UniquePipeline makeFullscreenPipeline(vk::PipelineShaderStageCreateInfo fragStage,
//	vk::PipelineColorBlendStateCreateInfo colorBlending, vk::PipelineLayout pipeLayout, vk::RenderPass renderPass,
//	uint32 subpassIndex)
//{
//	std::array stages {GpuAssetManager->}
//	return makePostProcPipeline;
//}
} // namespace rvk
