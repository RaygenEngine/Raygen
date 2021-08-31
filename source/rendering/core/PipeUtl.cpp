#include "PipeUtl.h"

#include "rendering/Device.h"

namespace rvk {
vk::UniquePipeline makeGraphicsPipeline(const std::vector<vk::PipelineShaderStageCreateInfo>& shaderStages,
	vk::PipelineVertexInputStateCreateInfo* pVertexInputState,
	vk::PipelineInputAssemblyStateCreateInfo* pInputAssemblyState,
	vk::PipelineTessellationStateCreateInfo* pTessellationState, vk::PipelineViewportStateCreateInfo* pViewportState,
	vk::PipelineRasterizationStateCreateInfo* pRasterizationState,
	vk::PipelineMultisampleStateCreateInfo* pMultisampleState,
	vk::PipelineDepthStencilStateCreateInfo* pDepthStencilState,
	vk::PipelineColorBlendStateCreateInfo* pColorBlendState, vk::PipelineDynamicStateCreateInfo* pDynamicState,
	vk::PipelineLayout pipeLayout, vk::RenderPass renderPass, uint32 subpassIndex, vk::PipelineCreateFlags flags,
	vk::Pipeline basePipeline, uint32 basePipelineIndex)
{
	// Defaults
	vk::PipelineColorBlendAttachmentState defaultBlendAttachment{};
	defaultBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
		.setBlendEnable(VK_FALSE);

	vk::PipelineColorBlendStateCreateInfo defaultColorBlendState{};
	defaultColorBlendState
		.setLogicOpEnable(VK_FALSE) //
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachments(defaultBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

	vk::PipelineVertexInputStateCreateInfo defaultVertexInputState{};

	vk::PipelineInputAssemblyStateCreateInfo defaultInputAssemblyState{};
	defaultInputAssemblyState
		.setTopology(vk::PrimitiveTopology::eTriangleList) //
		.setPrimitiveRestartEnable(VK_FALSE);

	// those are dynamic so they will be updated when needed
	vk::Viewport viewport{};
	vk::Rect2D scissor{};

	vk::PipelineViewportStateCreateInfo defaultViewportState{};
	defaultViewportState
		.setViewports(viewport) //
		.setScissors(scissor);

	vk::PipelineRasterizationStateCreateInfo defaultRasterizationState{};
	defaultRasterizationState
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

	vk::PipelineMultisampleStateCreateInfo defaultMultisampleState{};
	defaultMultisampleState
		.setSampleShadingEnable(VK_FALSE) //
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setMinSampleShading(1.f)
		.setPSampleMask(nullptr)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE);

	// dynamic states
	std::array dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
	};
	vk::PipelineDynamicStateCreateInfo defaultDynamicState{};
	defaultDynamicState.setDynamicStates(dynamicStates);

	// depth and stencil state
	vk::PipelineDepthStencilStateCreateInfo defaultDepthStencilState{};
	defaultDepthStencilState
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
		.setStages(shaderStages) //
		.setLayout(pipeLayout)
		.setRenderPass(renderPass)


		.setPTessellationState(pTessellationState)

		// our defaults - nullptr not accepted
		.setPVertexInputState(pVertexInputState ? pVertexInputState : &defaultVertexInputState)
		.setPInputAssemblyState(pInputAssemblyState ? pInputAssemblyState : &defaultInputAssemblyState)
		.setPViewportState(pViewportState ? pViewportState : &defaultViewportState)
		.setPRasterizationState(pRasterizationState ? pRasterizationState : &defaultRasterizationState)
		.setPMultisampleState(pMultisampleState ? pMultisampleState : &defaultMultisampleState)
		.setPDepthStencilState(pDepthStencilState ? pDepthStencilState : &defaultDepthStencilState)
		.setPColorBlendState(pColorBlendState ? pColorBlendState : &defaultColorBlendState)
		.setPDynamicState(pDynamicState ? pDynamicState : &defaultDynamicState)

		.setSubpass(subpassIndex)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1)
		.setFlags(flags);

	return vl::Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

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
	viewportState.setViewports(viewport).setScissors(scissor);

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
	std::array dynamicStates = {
		vk::DynamicState::eViewport,
		vk::DynamicState::eScissor,
	};
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.setDynamicStates(dynamicStates);

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
		.setStages(shaderStages) //
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

vk::UniquePipeline makeComputePipeline(
	const vk::PipelineShaderStageCreateInfo& shaderStage, vk::PipelineLayout pipeLayout)
{
	vk::ComputePipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStage(shaderStage) //
		.setLayout(pipeLayout)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	return vl::Device->createComputePipelineUnique(nullptr, pipelineInfo);
}

vk::UniquePipelineLayout makePipelineLayoutNoPC(std::vector<vk::DescriptorSetLayout> layoutsIn)
{
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(static_cast<uint32>(layoutsIn.size())) //
		.setPSetLayouts(layoutsIn.data())
		.setPushConstantRangeCount(0u)
		.setPPushConstantRanges(nullptr);

	return vl::Device->createPipelineLayoutUnique(pipelineLayoutInfo);
}

vk::UniquePipelineLayout makePipelineLayoutEx(
	std::initializer_list<vk::DescriptorSetLayout> layoutsIn, vk::ShaderStageFlags pcStageFlags, uint32 pcStructSize)
{
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(pcStageFlags) //
		.setSize(pcStructSize)
		.setOffset(0u);

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayouts(layoutsIn) //
		.setPushConstantRanges(pushConstantRange);

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
