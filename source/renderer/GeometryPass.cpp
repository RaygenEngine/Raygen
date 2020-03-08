#include "pch.h"
#include "renderer/GeometryPass.h"

#include "engine/Engine.h"
#include "engine/profiler/ProfileScope.h"
#include "renderer/VulkanLayer.h"

void GeometryPass::InitRenderPass()
{
	auto& device = VulkanLayer::device;
	vk::AttachmentDescription colorAttachment{};
	colorAttachment
		.setFormat(vk::Format::eR8G8B8A8Srgb) // CHECK:
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal); // CHECK:

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentDescription depthAttachment{};
	depthAttachment
		.setFormat(device->pd->FindDepthFormat()) //
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef
		.setAttachment(1u) //
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(1u)
		.setPColorAttachments(&colorAttachmentRef)
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::array attachments = { colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachmentCount(static_cast<uint32>(attachments.size())) //
		.setPAttachments(attachments.data())
		.setSubpassCount(1u)
		.setPSubpasses(&subpass)
		.setDependencyCount(1u)
		.setPDependencies(&dependency);

	m_renderPass = device->createRenderPassUnique(renderPassInfo);
}

void GeometryPass::InitFramebuffers()
{

	auto& device = VulkanLayer::device;
	vk::Extent2D fbSize = VulkanLayer::viewportFramebufferSize;


	// albedo buffer
	vk::Format format = vk::Format::eR8G8B8A8Srgb;

	albedoImage = std::make_unique<Image>(fbSize.width, fbSize.height, format, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal);

	albedoImage->TransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal);

	albedoImageView = albedoImage->RequestImageView2D_0_0();


	// depth buffer
	vk::Format depthFormat = device->pd->FindDepthFormat();

	depthImage = std::make_unique<Image>(fbSize.width, fbSize.height, depthFormat, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eDepthStencilAttachment, vk::MemoryPropertyFlagBits::eDeviceLocal);

	depthImage->TransitionToLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	depthImageView = depthImage->RequestImageView2D_0_0();


	// framebuffers

	std::array imAttachments = { albedoImageView.get(), depthImageView.get() };
	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(m_renderPass.get()) //
		.setAttachmentCount(static_cast<uint32>(imAttachments.size()))
		.setPAttachments(imAttachments.data())
		.setWidth(fbSize.width)
		.setHeight(fbSize.height)
		.setLayers(1);

	m_framebuffer = device->createFramebufferUnique(createInfo);
}

void GeometryPass::InitPipelineAndStuff()
{
	auto& device = VulkanLayer::device;
	auto& descriptorSetLayout = VulkanLayer::modelDescriptorSetLayout;

	// shaders
	auto vertShaderModule = device->CompileCreateShaderModule("engine-data/spv/gbuffer.vert");
	auto fragShaderModule = device->CompileCreateShaderModule("engine-data/spv/gbuffer.frag");


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

	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription
		.setBinding(0u) //
		.setStride(sizeof(VertexData))
		.setInputRate(vk::VertexInputRate::eVertex);

	std::array<vk::VertexInputAttributeDescription, 6> attributeDescriptions{};

	attributeDescriptions[0].binding = 0u;
	attributeDescriptions[0].location = 0u;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = offsetof(VertexData, position);

	attributeDescriptions[1].binding = 0u;
	attributeDescriptions[1].location = 1u;
	attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[1].offset = offsetof(VertexData, normal);

	attributeDescriptions[2].binding = 0u;
	attributeDescriptions[2].location = 2u;
	attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[2].offset = offsetof(VertexData, tangent);

	attributeDescriptions[3].binding = 0u;
	attributeDescriptions[3].location = 3u;
	attributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[3].offset = offsetof(VertexData, bitangent);

	attributeDescriptions[4].binding = 0u;
	attributeDescriptions[4].location = 4u;
	attributeDescriptions[4].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[4].offset = offsetof(VertexData, textCoord0);

	attributeDescriptions[5].binding = 0u;
	attributeDescriptions[5].location = 5u;
	attributeDescriptions[5].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[5].offset = offsetof(VertexData, textCoord1);

	vertexInputInfo
		.setVertexBindingDescriptionCount(1u) //
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
		.setPVertexBindingDescriptions(&bindingDescription)
		.setPVertexAttributeDescriptions(attributeDescriptions.data());

	vk::PipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly
		.setTopology(vk::PrimitiveTopology::eTriangleList) //
		.setPrimitiveRestartEnable(VK_FALSE);

	vk::Viewport viewport = GetViewport();
	vk::Rect2D scissor = GetScissor();

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
		.setFrontFace(vk::FrontFace::eCounterClockwise)
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

	vk::PipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment
		.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
						   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
		.setBlendEnable(VK_FALSE)
		.setSrcColorBlendFactor(vk::BlendFactor::eOne)
		.setDstColorBlendFactor(vk::BlendFactor::eZero)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
		.setAlphaBlendOp(vk::BlendOp::eAdd);

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending
		.setLogicOpEnable(VK_FALSE) //
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachmentCount(1u)
		.setPAttachments(&colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

	// Dynamic vieport
	vk::DynamicState dynamicStates[2] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo
		.setDynamicStateCount(2u) //
		.setPDynamicStates(&dynamicStates[0]);


	// pipeline layout
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
		.setSize(sizeof(glm::mat4))
		.setOffset(0u);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(1u) //
		.setPSetLayouts(&descriptorSetLayout.get())
		.setPushConstantRangeCount(1u)
		.setPPushConstantRanges(&pushConstantRange);

	m_pipelineLayout = device->createPipelineLayoutUnique(pipelineLayoutInfo);

	// depth and stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil
		.setDepthTestEnable(VK_TRUE) //
		.setDepthWriteEnable(VK_TRUE)
		.setDepthCompareOp(vk::CompareOp::eLess)
		.setDepthBoundsTestEnable(VK_FALSE)
		.setMinDepthBounds(0.0f) // Optional
		.setMaxDepthBounds(1.0f) // Optional
		.setStencilTestEnable(VK_FALSE)
		.setFront({}) // Optional
		.setBack({}); // Optional

	vk::GraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo
		.setStageCount(2u) //
		.setPStages(shaderStages)
		.setPVertexInputState(&vertexInputInfo)
		.setPInputAssemblyState(&inputAssembly)
		.setPViewportState(&viewportState)
		.setPRasterizationState(&rasterizer)
		.setPMultisampleState(&multisampling)
		.setPDepthStencilState(&depthStencil)
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(m_pipelineLayout.get())
		.setRenderPass(m_renderPass.get())
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	m_pipeline = device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}


void GeometryPass::RecordGeometryDraw(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);


	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer->begin(beginInfo);
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo
			.setRenderPass(m_renderPass.get()) //
			.setFramebuffer(m_framebuffer.get());
		renderPassInfo.renderArea
			.setOffset({ 0, 0 }) //
			.setExtent(VulkanLayer::viewportRect.extent);

		std::array<vk::ClearValue, 2> clearValues = {};
		clearValues[0].setColor(std::array{ 0.0f, 0.1f, 0.15f, 1.0f });
		clearValues[1].setDepthStencil({ 1.0f, 0 });
		renderPassInfo
			.setClearValueCount(static_cast<uint32>(clearValues.size())) //
			.setPClearValues(clearValues.data());

		// PERF: needs render pass?
		// begin render pass
		cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			// bind the graphics pipeline
			cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

			// Dynamic viewport & scissor
			cmdBuffer->setViewport(0, { GetViewport() });
			cmdBuffer->setScissor(0, { GetScissor() });

			for (auto& model : VulkanLayer::models) {
				for (auto& gg : model->geometryGroups) {
					// Submit via push constant (rather than a UBO)
					cmdBuffer->pushConstants(m_pipelineLayout.get(), vk::ShaderStageFlagBits::eVertex, 0u,
						sizeof(glm::mat4), &model->m_node->GetNodeTransformWCS());

					vk::Buffer vertexBuffers[] = { gg.vertexBuffer.get() };
					vk::DeviceSize offsets[] = { 0 };
					// geom
					cmdBuffer->bindVertexBuffers(0u, 1u, vertexBuffers, offsets);

					// indices
					cmdBuffer->bindIndexBuffer(gg.indexBuffer.get(), 0, vk::IndexType::eUint32);

					// descriptor sets
					cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u,
						&(gg.descriptorSet), 0u, nullptr);

					// draw call (triangle)
					cmdBuffer->drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
				}
			}
		}
		// end render pass
		cmdBuffer->endRenderPass();
	}
	// end command buffer recording
	cmdBuffer->end();
}

void GeometryPass::TransitionGBufferForShaderRead()
{
	auto& device = VulkanLayer::device;
	albedoImage->TransitionToLayout(vk::ImageLayout::eShaderReadOnlyOptimal);
}

void GeometryPass::TransitionGBufferForAttachmentWrite()
{
	auto& device = VulkanLayer::device;
	albedoImage->TransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal);
}

vk::Viewport GeometryPass::GetViewport() const
{
	auto vpSize = VulkanLayer::viewportRect.extent;

	vk::Viewport viewport{};
	viewport
		.setX(0) //
		.setY(static_cast<float>(vpSize.height))
		.setWidth(static_cast<float>(vpSize.width))
		.setHeight(-static_cast<float>(vpSize.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);
	return viewport;
}

vk::Rect2D GeometryPass::GetScissor() const
{
	vk::Rect2D scissor{};

	scissor
		.setOffset({ 0, 0 }) //
		.setExtent(VulkanLayer::viewportRect.extent);

	return scissor;
}
