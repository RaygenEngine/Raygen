#include "pch/pch.h"

#include "renderer/DeferredPass.h"
#include "renderer/VulkanLayer.h"
#include "editor/imgui/ImguiImpl.h"
#include "system/Engine.h"

void DeferredPass::InitRenderPassAndFramebuffers()
{
	auto& swapchain = VulkanLayer::swapchain;
	auto& device = VulkanLayer::device;

	vk::AttachmentDescription colorAttachment{};
	colorAttachment.setFormat(swapchain->imageFormat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::ePresentSrcKHR);


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
		.setColorAttachmentCount(1)
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

	m_framebuffers.clear();
	m_framebuffers.resize(swapchain->images.size());

	// framebuffers
	for (auto i = 0; i < swapchain->images.size(); ++i) {
		std::array<vk::ImageView, 2> attachments = { swapchain->imageViews[i].get(), swapchain->depthImageView.get() };
		vk::FramebufferCreateInfo createInfo{};
		createInfo.setRenderPass(m_renderPass.get())
			.setAttachmentCount(static_cast<uint32>(attachments.size()))
			.setPAttachments(attachments.data())
			.setWidth(swapchain->extent.width)
			.setHeight(swapchain->extent.height)
			.setLayers(1);

		m_framebuffers[i] = device->handle->createFramebufferUnique(createInfo);
	}
}

void DeferredPass::InitPipelineAndStuff()
{
	auto& device = VulkanLayer::device;
	auto& swapchain = VulkanLayer::swapchain;
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

	vk::Viewport viewport{};
	viewport.setX(static_cast<float>(g_ViewportCoordinates.position.x))
		.setY(static_cast<float>(g_ViewportCoordinates.position.y + g_ViewportCoordinates.size.y))
		.setWidth(static_cast<float>(g_ViewportCoordinates.size.x))
		.setHeight(-static_cast<float>(g_ViewportCoordinates.size.y))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);

	vk::Rect2D scissor{};
	scissor.setOffset({ 0, 0 });
	// WIP: is this correct?
	scissor.setExtent(swapchain->extent);

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

	std::array layouts = { descriptorSetLayout.get() };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayoutCount(1u)
		.setPSetLayouts(layouts.data())
		.setPushConstantRangeCount(0u)
		.setPPushConstantRanges(&pushConstantRange);

	m_pipelineLayout = device->handle->createPipelineLayoutUnique(pipelineLayoutInfo);

	// depth and stencil state
	vk::PipelineDepthStencilStateCreateInfo depthStencil{}; // WIP
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


void DeferredPass::RecordCmd(vk::CommandBuffer* cmdBuffer, vk::Framebuffer& framebuffer)
{
	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer->begin(beginInfo);
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.setRenderPass(m_renderPass.get()).setFramebuffer(framebuffer);
		// WIP: extent
		renderPassInfo.renderArea.setOffset({ 0, 0 }).setExtent(
			vk::Extent2D{ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y });
		std::array<vk::ClearValue, 2> clearValues = {};
		clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[1].setDepthStencil({ 1.0f, 0 });
		renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
		renderPassInfo.setPClearValues(clearValues.data());

		// WIP: render pass doesn't start from here
		// begin render pass
		cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			// bind the graphics pipeline
			cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

			// descriptor sets
			cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u,
				&(VulkanLayer::quadDescriptorSet.get()), 0u, nullptr);

			// draw call (triangle)
			cmdBuffer->draw(3u, 1u, 0u, 0u);
		}
		ImguiImpl::RenderVulkan(cmdBuffer);

		// end render pass
		cmdBuffer->endRenderPass();
	}

	// end command buffer recording
	cmdBuffer->end();
}
