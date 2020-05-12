#include "pch.h"
#include "PtBase.h"

#include "rendering/Swapchain.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"

namespace vl {

void PtBase_SinglePipeline::Utl_CreatePipeline(
	GpuAsset<Shader>& shader, vk::PipelineColorBlendStateCreateInfo colorBlending)
{
	std::vector shaderStages = shader.shaderStages; /// AUTO


	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly
		.setTopology(vk::PrimitiveTopology::eTriangleList) //
		.setPrimitiveRestartEnable(VK_FALSE);

	// those are dynamic so they will be updated when needed
	vk::Viewport viewport{};
	vk::Rect2D scissor{};

	vk::PipelineViewportStateCreateInfo viewportState{}; /// STATIC
	viewportState
		.setViewportCount(1u) //
		.setPViewports(&viewport)
		.setScissorCount(1u)
		.setPScissors(&scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer{}; /// STATIC
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

	vk::PipelineMultisampleStateCreateInfo multisampling{}; /// STATIC
	multisampling
		.setSampleShadingEnable(VK_FALSE) //
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setMinSampleShading(1.f)
		.setPSampleMask(nullptr)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE);

	// vk::PipelineColorBlendAttachmentState colorBlendAttachment{}; /// CUSTOM
	// colorBlendAttachment
	//	.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
	//					   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
	//	.setBlendEnable(VK_TRUE)
	//	.setSrcColorBlendFactor(vk::BlendFactor::eOne)
	//	.setDstColorBlendFactor(vk::BlendFactor::eOne)
	//	.setColorBlendOp(vk::BlendOp::eAdd)
	//	.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
	//	.setDstAlphaBlendFactor(vk::BlendFactor::eOne)
	//	.setAlphaBlendOp(vk::BlendOp::eAdd);

	// vk::PipelineColorBlendStateCreateInfo colorBlending{};
	// colorBlending
	//	.setLogicOpEnable(VK_FALSE) //
	//	.setLogicOp(vk::LogicOp::eCopy)
	//	.setAttachmentCount(1u)
	//	.setPAttachments(&colorBlendAttachment)
	//	.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });


	// dynamic states
	vk::DynamicState dynamicStates[] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor }; /// STATIC
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

	vk::GraphicsPipelineCreateInfo pipelineInfo{}; /// AUTO
	pipelineInfo
		.setStageCount(static_cast<uint32>(shaderStages.size())) //
		.setPStages(shaderStages.data())
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(m_pipelineLayout.get())
		.setRenderPass(*Renderer->m_ptRenderpass) // WIP: Proper renderpass
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	m_pipeline = Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}
} // namespace vl
