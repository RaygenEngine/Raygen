#include "pch/pch.h"

#include "renderer/GeometryPass.h"
#include "renderer/VulkanLayer.h"
#include "system/Engine.h"

void GeometryPass::CreateFramebufferImageViews()
{
}

void GeometryPass::InitRenderPassAndFramebuffers()
{
	auto& device = VulkanLayer::device;


	vk::AttachmentDescription colorAttachment{};
	colorAttachment
		.setFormat(vk::Format::eR8G8B8A8Srgb) // WIP
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal); // WIP

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef.setAttachment(0);
	colorAttachmentRef.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentDescription depthAttachment{};
	depthAttachment.setFormat(device->pd->FindDepthFormat())
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef.setAttachment(1);
	depthAttachmentRef.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics);
	subpass.setColorAttachmentCount(1)
		.setPColorAttachments(&colorAttachmentRef)
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL)
		.setDstSubpass(0)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::array<vk::AttachmentDescription, 2> attachments = { colorAttachment, depthAttachment };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo.setAttachmentCount(static_cast<uint32>(attachments.size()))
		.setPAttachments(attachments.data())
		.setSubpassCount(1)
		.setPSubpasses(&subpass)
		.setDependencyCount(1)
		.setPDependencies(&dependency);

	m_renderPass = device->handle->createRenderPassUnique(renderPassInfo);

	// albedo buffer
	vk::Format format = vk::Format::eR8G8B8A8Srgb;

	device->CreateImage(g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y, format, vk::ImageTiling::eOptimal,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
		vk::MemoryPropertyFlagBits::eDeviceLocal, albedoImage, albedoImageMemory);

	// WIP:
	// device->TransitionImageLayout(
	//	albedoImage.get(), format, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

	vk::ImageViewCreateInfo viewInfo{};
	viewInfo.setImage(albedoImage.get()).setViewType(vk::ImageViewType::e2D).setFormat(format);
	viewInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eColor)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	albedoImageView = device->handle->createImageViewUnique(viewInfo);

	// depth buffer
	vk::Format depthFormat = device->pd->FindDepthFormat();
	device->CreateImage(g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y, depthFormat,
		vk::ImageTiling::eOptimal, vk::ImageUsageFlagBits::eDepthStencilAttachment,
		vk::MemoryPropertyFlagBits::eDeviceLocal, depthImage, depthImageMemory);

	device->TransitionImageLayout(
		depthImage.get(), depthFormat, vk::ImageLayout::eUndefined, vk::ImageLayout::eDepthStencilAttachmentOptimal);

	viewInfo.setImage(depthImage.get()).setViewType(vk::ImageViewType::e2D).setFormat(depthFormat);
	viewInfo.subresourceRange.setAspectMask(vk::ImageAspectFlagBits::eDepth)
		.setBaseMipLevel(0)
		.setLevelCount(1)
		.setBaseArrayLayer(0)
		.setLayerCount(1);

	depthImageView = device->handle->createImageViewUnique(viewInfo);


	// framebuffers

	std::array<vk::ImageView, 2> imAttachments = { albedoImageView.get(), depthImageView.get() };
	vk::FramebufferCreateInfo createInfo{};
	createInfo.setRenderPass(m_renderPass.get())
		.setAttachmentCount(static_cast<uint32>(imAttachments.size()))
		.setPAttachments(imAttachments.data())
		.setWidth(g_ViewportCoordinates.size.x)
		.setHeight(g_ViewportCoordinates.size.y)
		.setLayers(1);

	m_framebuffer = device->handle->createFramebufferUnique(createInfo);
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

	vk::Viewport viewport{};
	viewport.setX(0)
		.setY((float)g_ViewportCoordinates.size.y)
		.setWidth((float)g_ViewportCoordinates.size.x)
		.setHeight(-(float)(g_ViewportCoordinates.size.y))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);

	vk::Rect2D scissor{};
	scissor.setOffset({ 0, 0 });
	scissor.setExtent({ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y });

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
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(sizeof(glm::mat4)).setOffset(0u);

	std::array layouts = { descriptorSetLayout.get() };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayoutCount(1u)
		.setPSetLayouts(layouts.data())
		.setPushConstantRangeCount(1u)
		.setPPushConstantRanges(&pushConstantRange);

	m_pipelineLayout = device->handle->createPipelineLayoutUnique(pipelineLayoutInfo);

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
		.setRenderPass(m_renderPass.get())
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	m_pipeline = device->handle->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}


void GeometryPass::RecordGeometryDraw(vk::CommandBuffer* cmdBuffer)
{
	// PROFILE_SCOPE(GE);
	// WIP

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer->begin(beginInfo);
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.setRenderPass(m_renderPass.get()).setFramebuffer(m_framebuffer.get());
		// WIP: extent
		renderPassInfo.renderArea.setOffset({ 0, 0 }).setExtent(
			vk::Extent2D{ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y });
		std::array<vk::ClearValue, 2> clearValues = {};
		clearValues[0].setColor(std::array{ 0.0f, 0.1f, 0.15f, 1.0f });
		clearValues[1].setDepthStencil({ 1.0f, 0 });
		renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
		renderPassInfo.setPClearValues(clearValues.data());

		// WIP: render pass doesn't start from here
		// begin render pass
		cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			// bind the graphics pipeline
			cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());


			for (auto& model : VulkanLayer::models) {
				for (auto& gg : model->geometryGroups) {
					// PROFILE_SCOPE(Renderer);

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
	device->TransitionImageLayout(albedoImage.get(), vk::Format::eR8G8B8A8Srgb,
		vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
}

void GeometryPass::TransitionGBufferForAttachmentWrite()
{
	auto& device = VulkanLayer::device;
	device->TransitionImageLayout(albedoImage.get(), vk::Format::eR8G8B8A8Srgb, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::ImageLayout::eColorAttachmentOptimal);
}
