#include "pch/pch.h"

#include "renderer/renderers/vulkan/GraphicsPipeline.h"
#include "asset/pods/ModelPod.h"

namespace vlkn {
GraphicsPipeline::GraphicsPipeline(Device* device, Swapchain* swapchain)
{
	// descriptor layout

	vk::DescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.setBinding(0u)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1u)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.setBindingCount(1u).setPBindings(&uboLayoutBinding);

	m_descriptorSetLayout = device->createDescriptorSetLayoutUnique(layoutInfo);

	// shaders
	auto vertShaderModule = device->CreateShaderModule("/engine-data/spv/vert.spv");
	auto fragShaderModule = device->CreateShaderModule("/engine-data/spv/frag.spv");

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.setStage(vk::ShaderStageFlagBits::eVertex).setModule(vertShaderModule.get()).setPName("main");

	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.setStage(vk::ShaderStageFlagBits::eFragment).setModule(fragShaderModule.get()).setPName("main");

	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription.setBinding(0u).setStride(sizeof(VertexData)).setInputRate(vk::VertexInputRate::eVertex);

	std::array<vk::VertexInputAttributeDescription, 6> attributeDescriptions{};

	attributeDescriptions[0].binding = 0;
	attributeDescriptions[0].location = 0;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = offsetof(VertexData, position);

	attributeDescriptions[1].binding = 0;
	attributeDescriptions[1].location = 1;
	attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[1].offset = offsetof(VertexData, normal);

	attributeDescriptions[2].binding = 0;
	attributeDescriptions[2].location = 2;
	attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[2].offset = offsetof(VertexData, tangent);

	attributeDescriptions[3].binding = 0;
	attributeDescriptions[3].location = 3;
	attributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[3].offset = offsetof(VertexData, bitangent);

	attributeDescriptions[4].binding = 0;
	attributeDescriptions[4].location = 4;
	attributeDescriptions[4].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[4].offset = offsetof(VertexData, textCoord0);

	attributeDescriptions[5].binding = 0;
	attributeDescriptions[5].location = 5;
	attributeDescriptions[5].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[5].offset = offsetof(VertexData, textCoord1);

	vertexInputInfo.setVertexBindingDescriptionCount(1u)
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
		.setPVertexBindingDescriptions(&bindingDescription)
		.setPVertexAttributeDescriptions(attributeDescriptions.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.setTopology(vk::PrimitiveTopology::eTriangleList).setPrimitiveRestartEnable(VK_FALSE);

	auto ext = swapchain->GetExtent();
	vk::Viewport viewport{};
	viewport.setX(0.f)
		.setY(static_cast<float>(ext.height))
		.setWidth(static_cast<float>(ext.width))
		.setHeight(-static_cast<float>(ext.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);

	vk::Rect2D scissor{};
	scissor.setOffset({ 0, 0 });
	scissor.setExtent(ext);

	vk::PipelineViewportStateCreateInfo viewportState{};
	viewportState.setViewportCount(1u).setPViewports(&viewport).setScissorCount(1u).setPScissors(&scissor);

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.setDepthClampEnable(VK_FALSE)
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eBack)
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
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayoutCount(1u);
	pipelineLayoutInfo.setPSetLayouts(&(m_descriptorSetLayout.get()));
	pipelineLayoutInfo.setPushConstantRangeCount(0u);
	pipelineLayoutInfo.setPPushConstantRanges(nullptr);

	m_pipelineLayout = device->createPipelineLayoutUnique(pipelineLayoutInfo);

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
		.setPDynamicState(nullptr)
		.setLayout(m_pipelineLayout.get())
		.setRenderPass(swapchain->GetRenderPass())
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	m_pipeline = device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}
} // namespace vlkn
