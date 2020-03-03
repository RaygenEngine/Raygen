#include "pch/pch.h"

#include "system/profiler/ProfileScope.h"
#include "renderer/DeferredPass.h"
#include "renderer/VulkanLayer.h"
#include "editor/imgui/ImguiImpl.h"
#include "system/Engine.h"

void DeferredPass::InitPipeline(vk::RenderPass renderPass)
{
	auto& device = VulkanLayer::device;
	auto& descriptorSetLayout = VulkanLayer::quadDescriptorSetLayout;

	// shaders
	auto vertShaderModule = device->CompileCreateShaderModule("engine-data/spv/deferred.vert");
	auto fragShaderModule = device->CompileCreateShaderModule("engine-data/spv/deferred.frag");


	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eVertex) //
		.setModule(vertShaderModule.get())
		.setPName("main");

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eFragment) //
		.setModule(fragShaderModule.get())
		.setPName("main");

	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(VK_FALSE);


	const float x = static_cast<float>(g_ViewportCoordinates.position.x);
	const float y = static_cast<float>(g_ViewportCoordinates.position.y);
	const float width = static_cast<float>(g_ViewportCoordinates.size.x);
	const float height = static_cast<float>(g_ViewportCoordinates.size.y);

	vk::Viewport viewport{};
	viewport.setX(x).setY(y).setWidth(width).setHeight(height).setMinDepth(0.f).setMaxDepth(1.f);

	vk::Extent2D ext;
	ext.setWidth(g_ViewportCoordinates.size.x).setHeight(g_ViewportCoordinates.size.y);

	vk::Rect2D scissor{};
	scissor.setOffset(vk::Offset2D(g_ViewportCoordinates.position.x, g_ViewportCoordinates.position.y)).setExtent(ext);

	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState.setViewportCount(1u).setPViewports(&viewport).setScissorCount(1u).setPScissors(&scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eFront) // TODO: RENDERER Fix in shader
		.setFrontFace(vk::FrontFace::eCounterClockwise)
		.setDepthBiasEnable(VK_FALSE)
		.setDepthBiasConstantFactor(0.f)
		.setDepthBiasClamp(0.f)
		.setDepthBiasSlopeFactor(0.f);

	vk::PipelineMultisampleStateCreateInfo multisampling{};
	multisampling.setSampleShadingEnable(VK_FALSE)
		.setRasterizationSamples(vk::SampleCountFlagBits::e1)
		.setMinSampleShading(1.f)
		.setPSampleMask(nullptr)
		.setAlphaToCoverageEnable(VK_FALSE)
		.setAlphaToOneEnable(VK_FALSE);

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA)
		.setBlendEnable(VK_FALSE)
		.setSrcColorBlendFactor(vk::BlendFactor::eOne)
		.setDstColorBlendFactor(vk::BlendFactor::eZero)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
		.setAlphaBlendOp(vk::BlendOp::eAdd);

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.setLogicOpEnable(VK_FALSE)
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachmentCount(1u)
		.setPAttachments(&colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });


	// dynamic states
	vk::DynamicState dynamicStates[] = { vk::DynamicState::eViewport, vk::DynamicState::eLineWidth };

	vk::PipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.setDynamicStateCount(2u).setPDynamicStates(dynamicStates);

	// pipeline layout
	vk::PushConstantRange pushConstantRange{};

	std::array layouts = { descriptorSetLayout.get() };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayoutCount(1u)
		.setPSetLayouts(layouts.data())
		.setPushConstantRangeCount(0u)
		.setPPushConstantRanges(&pushConstantRange);

	m_pipelineLayout = device->createPipelineLayoutUnique(pipelineLayoutInfo);

	// TODO: remove depth from deferred pass
	// depth and stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.setDepthTestEnable(VK_TRUE);
	depthStencil.setDepthWriteEnable(VK_TRUE);
	depthStencil.setDepthCompareOp(vk::CompareOp::eLess);
	depthStencil.setDepthBoundsTestEnable(VK_FALSE);
	depthStencil.setMinDepthBounds(0.0f); // Optional
	depthStencil.setMaxDepthBounds(1.0f); // Optional
	depthStencil.setStencilTestEnable(VK_FALSE);
	depthStencil.setFront({}); // Optional
	depthStencil.setBack({});  // Optional

	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.setStageCount(2u)
		.setPStages(shaderStages)
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(nullptr) // TODO: check
		.setLayout(m_pipelineLayout.get())
		.setRenderPass(renderPass)
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	m_pipeline = device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}


void DeferredPass::RecordCmd(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);

	// bind the graphics pipeline
	cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

	// descriptor sets
	cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u,
		&(VulkanLayer::quadDescriptorSet.get()), 0u, nullptr);

	// draw call (triangle)
	cmdBuffer->draw(3u, 1u, 0u, 0u);
}
