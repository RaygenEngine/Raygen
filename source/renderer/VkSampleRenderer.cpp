#include "pch/pch.h"

#include "renderer/VkSampleRenderer.h"
#include "system/Logger.h"
#include "system/Engine.h"


#include "renderer/VulkanLayer.h"
#include "asset/AssetManager.h"
#include "world/World.h"
#include "world/nodes/geometry/GeometryNode.h"
#include "world/nodes/camera/CameraNode.h"
#include "editor/imgui/ImguiImpl.h"
#include "system/profiler/ProfileScope.h"

#include <set>


VkSampleRenderer::~VkSampleRenderer()
{
	/*m_device->waitIdle();

	m_graphicsPipeline.reset();
	m_swapchain.reset();
	m_descriptors.reset();*/

	m_models.clear();


	Engine::Get().m_remakeWindow = true;
}

void VkSampleRenderer::InitWorld()
{
	auto world = Engine::GetWorld();
	m_models.clear();
	for (auto geomNode : world->GetNodeIterator<GeometryNode>()) {

		auto model = geomNode->GetModel();
		m_models.emplace_back(model);
		m_models.back()->m_transform = geomNode->GetNodeTransformWCS();
	}
}

void VkSampleRenderer::Init()
{
	m_resizeListener.Bind([&](auto, auto) { m_shouldRecreateSwapchain = true; });
	m_worldLoaded.Bind([&]() { InitWorld(); });
	m_viewportUpdated.Bind([&]() { m_shouldRecreatePipeline = true; });

	InitRenderPassAndFramebuffersOfCurrentSwapchain();
	InitGraphicsPipeline();
	InitRenderCmdBuffers();
	InitUniformBuffers();

	// semaphores
	auto device = VulkanLayer::GetDevice();
	m_imageAvailableSemaphore = device->handle->createSemaphoreUnique({});
	m_renderFinishedSemaphore = device->handle->createSemaphoreUnique({});

	::ImguiImpl::InitVulkan();
} // namespace vk

void VkSampleRenderer::InitRenderPassAndFramebuffersOfCurrentSwapchain()
{
	auto swapchain = VulkanLayer::GetSwapchain();
	auto device = VulkanLayer::GetDevice();

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

void VkSampleRenderer::InitGraphicsPipeline()
{
	auto device = VulkanLayer::GetDevice();
	auto swapchain = VulkanLayer::GetSwapchain();

	// descriptor layout

	vk::DescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.setBinding(0u)
		.setDescriptorType(vk::DescriptorType::eUniformBuffer)
		.setDescriptorCount(1u)
		.setStageFlags(vk::ShaderStageFlagBits::eVertex)
		.setPImmutableSamplers(nullptr);

	vk::DescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.setBinding(1u)
		.setDescriptorCount(1u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setPImmutableSamplers(nullptr)
		.setStageFlags(vk::ShaderStageFlagBits::eFragment);

	std::array<vk::DescriptorSetLayoutBinding, 2> bindings{ uboLayoutBinding, samplerLayoutBinding };
	vk::DescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.setBindingCount(static_cast<uint32>(bindings.size())).setPBindings(bindings.data());

	m_descriptorSetLayout = device->handle->createDescriptorSetLayoutUnique(layoutInfo);

	// shaders
	auto vertShaderModule = device->CompileCreateShaderModule("engine-data/spv/test.vert");
	auto fragShaderModule = device->CompileCreateShaderModule("engine-data/spv/test.frag");


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
	pushConstantRange.setStageFlags(vk::ShaderStageFlagBits::eVertex).setSize(sizeof(glm::mat4)).setOffset(0u);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayoutCount(1u)
		.setPSetLayouts(&(m_descriptorSetLayout.get()))
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

void VkSampleRenderer::InitRenderCmdBuffers()
{
	auto device = VulkanLayer::GetDevice();
	auto swapchain = VulkanLayer::GetSwapchain();

	m_renderCmdBuffers.clear();

	m_renderCmdBuffers.resize(swapchain->images.size());

	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(static_cast<uint32>(m_renderCmdBuffers.size()));

	m_renderCmdBuffers = device->handle->allocateCommandBuffersUnique(allocInfo);
}

void VkSampleRenderer::InitUniformBuffers()
{
	auto device = VulkanLayer::GetDevice();
	auto swapchain = VulkanLayer::GetSwapchain();

	m_uniformBuffers.clear();
	m_uniformBuffersMemory.clear();

	m_uniformBuffers.resize(swapchain->images.size());
	m_uniformBuffersMemory.resize(swapchain->images.size());

	vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

	for (size_t i = 0; i < swapchain->images.size(); i++) {
		device->CreateBuffer(bufferSize, vk::BufferUsageFlagBits::eUniformBuffer,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent, m_uniformBuffers[i],
			m_uniformBuffersMemory[i]);
	}
}

void VkSampleRenderer::RecordCommandBuffer(int32 imageIndex)
{
	PROFILE_SCOPE(Renderer);

	auto& cmdBuffer = m_renderCmdBuffers[imageIndex];
	// WIP

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);

	// begin command buffer recording
	cmdBuffer->begin(beginInfo);
	{
		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo.setRenderPass(m_renderPass.get()).setFramebuffer(m_framebuffers[imageIndex].get());
		// WIP: extent
		renderPassInfo.renderArea.setOffset({ 0, 0 }).setExtent(VulkanLayer::GetSwapchain()->extent);
		std::array<vk::ClearValue, 2> clearValues = {};
		clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[1].setDepthStencil({ 1.0f, 0 });
		renderPassInfo.setClearValueCount(static_cast<uint32>(clearValues.size()));
		renderPassInfo.setPClearValues(clearValues.data());

		// begin render pass
		cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			// bind the graphics pipeline
			cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());


			for (auto& model : m_models) {
				// Submit via push constant (rather than a UBO)
				cmdBuffer->pushConstants(m_pipelineLayout.get(), vk::ShaderStageFlagBits::eVertex, 0u,
										 sizeof(glm::mat4), &model->m_transform);

				for (auto& gg : model->geometryGroups) {
					PROFILE_SCOPE_CHEAP(Renderer);

					vk::Buffer vertexBuffers[] = { gg.vertexBuffer.get() };
					vk::DeviceSize offsets[] = { 0 };
					// geom
					cmdBuffer->bindVertexBuffers(0u, 1u, vertexBuffers, offsets);

					// indices
					cmdBuffer->bindIndexBuffer(gg.indexBuffer.get(), 0, vk::IndexType::eUint32);

					// descriptor sets
					cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u,
						&(gg.descriptorSets[imageIndex]), 0u, nullptr);

					// draw call (triangle)
					cmdBuffer->drawIndexed(gg.indexCount, 1u, 0u, 0u, 0u);
				}
			}

			::ImguiImpl::RenderVulkan(&cmdBuffer.get());
		}
		// end render pass
		cmdBuffer->endRenderPass();
	}
	// end command buffer recording
	cmdBuffer->end();
}

void VkSampleRenderer::DrawFrame()
{
	PROFILE_SCOPE(Renderer);

	if (m_shouldRecreateSwapchain) {
		// m_device->waitIdle();

		// m_descriptors.reset();
		// m_graphicsPipeline.reset();
		// m_swapchain.reset();

		// m_device->freeCommandBuffers(m_device.GetGraphicsCommandPool(), m_renderCommandBuffers);

		// m_swapchain = m_device.RequestDeviceSwapchainOnSurface(m_instance.GetSurface());
		// m_graphicsPipeline = m_device.RequestDeviceGraphicsPipeline(m_swapchain.get());
		// m_descriptors = m_device.RequestDeviceDescriptors(m_swapchain.get(), m_graphicsPipeline.get());

		// m_models.clear();
		// CreateGeometry();

		// m_renderCommandBuffers.clear();
		// AllocateRenderCommandBuffers();

		m_shouldRecreateSwapchain = false;
		m_shouldRecreatePipeline = false;
		::ImguiImpl::InitVulkan();
	}
	else if (m_shouldRecreatePipeline) {
		m_shouldRecreatePipeline = false;

		// m_descriptors.reset();
		// m_graphicsPipeline.reset();


		// m_graphicsPipeline = m_device.RequestDeviceGraphicsPipeline(m_swapchain.get());
		// m_descriptors = m_device.RequestDeviceDescriptors(m_swapchain.get(), m_graphicsPipeline.get());

		// m_models.clear();
		// CreateGeometry();

		::ImguiImpl::InitVulkan();
	}

	uint32 imageIndex;

	vk::Result result0 = m_device->acquireNextImageKHR(
		m_swapchain->Get(), UINT64_MAX, m_imageAvailableSemaphore.get(), {}, &imageIndex);

	switch (result0) {
		case vk::Result::eErrorOutOfDateKHR: return;
		case vk::Result::eSuccess:
		case vk::Result::eSuboptimalKHR: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}

	// WIP: UNIFORM BUFFER UPDATES
	{
		auto world = Engine::GetWorld();
		auto camera = world->GetActiveCamera();

		UniformBufferObject ubo{};
		ubo.view = camera->GetViewMatrix();
		ubo.proj = camera->GetProjectionMatrix();

		void* data = m_device->mapMemory(m_descriptors->GetUniformBuffersMemory()[imageIndex], 0, sizeof(ubo));
		memcpy(data, &ubo, sizeof(ubo));
		m_device->unmapMemory(m_descriptors->GetUniformBuffersMemory()[imageIndex]);
	}

	vk::SubmitInfo submitInfo{};
	vk::Semaphore waitSemaphores[] = { m_imageAvailableSemaphore.get() };


	RecordCommandBuffer(imageIndex);

	// wait with writing colors to the image until it's available
	// the implementation can already start executing our vertex shader and such while the image is not yet
	// available
	vk::PipelineStageFlags waitStages[] = { vk::PipelineStageFlagBits::eColorAttachmentOutput };

	submitInfo.setWaitSemaphoreCount(1u)
		.setPWaitSemaphores(waitSemaphores)
		.setPWaitDstStageMask(waitStages)
		.setCommandBufferCount(1u)
		.setPCommandBuffers(&m_renderCmdBuffers[imageIndex].get());

	// which semaphores to signal once the command buffer(s) have finished execution
	vk::Semaphore signalSemaphores[] = { m_renderFinishedSemaphore.get() };
	submitInfo.setSignalSemaphoreCount(1u).setPSignalSemaphores(signalSemaphores);

	m_device.GetGraphicsQueue()->submit(1u, &submitInfo, {});
	vk::PresentInfoKHR presentInfo;
	presentInfo.setWaitSemaphoreCount(1u).setPWaitSemaphores(signalSemaphores);

	vk::SwapchainKHR swapChains[] = { m_swapchain->Get() };
	presentInfo.setSwapchainCount(1u).setPSwapchains(swapChains).setPImageIndices(&imageIndex).setPResults(nullptr);

	vk::Result result1 = m_device.GetPresentQueue()->presentKHR(presentInfo);

	m_device.GetPresentQueue()->waitIdle();

	switch (result1) {
		case vk::Result::eErrorOutOfDateKHR:
		case vk::Result::eSuboptimalKHR: return;
		case vk::Result::eSuccess: break;
		default: LOG_ABORT("failed to acquire swap chain image!");
	}
}
