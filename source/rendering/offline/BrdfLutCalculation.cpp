#include "pch.h"
#include "BrdfLutCalculation.h"

#include "assets/AssetRegistry.h"
#include "assets/PodEditor.h"
#include "assets/pods/EnvironmentMap.h"
#include "assets/pods/Image.h"
#include "engine/console/ConsoleVariable.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/scene/Scene.h"
#include "rendering/wrappers/Buffer.h"

namespace vl {
BrdfLutCalculation::BrdfLutCalculation(GpuEnvironmentMap* envmapAsset, uint32 calculationResolution)
	: m_envmapAsset(envmapAsset)
	, m_resolution(calculationResolution)
{
}

void BrdfLutCalculation::Calculate()
{
	MakeRenderPass();

	AllocateCommandBuffers();

	MakePipeline();

	PrepareFaceInfo();

	RecordAndSubmitCmdBuffers();

	EditPods();
}

void BrdfLutCalculation::MakeRenderPass()
{
	vk::AttachmentDescription colorAttachmentDesc{};
	colorAttachmentDesc
		.setFormat(vk::Format::eR32G32B32A32Sfloat) //
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal)
		.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference colorAttachmentRef{};
	colorAttachmentRef
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(1u)
		.setPColorAttachments(&colorAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::array attachments{ colorAttachmentDesc };
	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachmentCount(static_cast<uint32>(attachments.size())) //
		.setPAttachments(attachments.data())
		.setSubpassCount(1u)
		.setPSubpasses(&subpass)
		.setDependencyCount(1u)
		.setPDependencies(&dependency);

	m_renderPass = Device->createRenderPassUnique(renderPassInfo);
}

void BrdfLutCalculation::AllocateCommandBuffers()
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(1u);

	m_cmdBuffer = Device->allocateCommandBuffers(allocInfo)[0];
}

void BrdfLutCalculation::MakePipeline()
{
	auto& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/offline/brdflut.shader");

	if (!gpuShader.HasValidModule()) {
		LOG_ERROR("Geometry Pipeline skipped due to shader compilation errors.");
		return;
	}
	std::vector shaderStages = gpuShader.shaderStages;

	auto& fragShaderModule = gpuShader.frag;
	auto& vertShaderModule = gpuShader.vert;

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

	static ConsoleVariable<vk::PolygonMode> fillmode{ "brdfPolygonmode", vk::PolygonMode::eFill };

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer
		.setDepthClampEnable(VK_FALSE) //
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(fillmode)
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

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(0u) //
		.setPSetLayouts(nullptr)
		.setPushConstantRangeCount(0u)
		.setPPushConstantRanges(nullptr);

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);

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
		.setPColorBlendState(&colorBlending)
		.setPDynamicState(&dynamicStateInfo)
		.setLayout(m_pipelineLayout.get())
		.setRenderPass(m_renderPass.get())
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	m_pipeline = Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

void BrdfLutCalculation::PrepareFaceInfo()
{
	m_attachment = RImageAttachment{ m_resolution, m_resolution, vk::Format::eR32G32B32A32Sfloat,
		vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
		vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eDeviceLocal, "face" };
	m_attachment.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(m_renderPass.get()) //
		.setAttachmentCount(1u)
		.setPAttachments(&m_attachment())
		.setWidth(m_resolution)
		.setHeight(m_resolution)
		.setLayers(1);

	m_framebuffer = Device->createFramebufferUnique(createInfo);
}

void BrdfLutCalculation::RecordAndSubmitCmdBuffers()
{
	Device->waitIdle();

	PROFILE_SCOPE(Renderer);

	vk::Rect2D scissor{};

	scissor
		.setOffset({ 0, 0 }) //
		.setExtent({ m_resolution, m_resolution });

	vk::Viewport viewport{};

	viewport
		.setWidth(static_cast<float>(m_resolution)) //
		.setHeight(static_cast<float>(m_resolution));


	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(m_renderPass.get()) //
		.setFramebuffer(m_framebuffer.get());
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(scissor.extent);

	std::array<vk::ClearValue, 1> clearValues = {};
	clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	renderPassInfo
		.setClearValueCount(static_cast<uint32>(clearValues.size())) //
		.setPClearValues(clearValues.data());

	vk::CommandBufferBeginInfo beginInfo{};
	beginInfo.setFlags(vk::CommandBufferUsageFlags(0)).setPInheritanceInfo(nullptr);
	m_cmdBuffer.begin(beginInfo);
	{
		// begin render pass
		m_cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			// Dynamic viewport & scissor
			m_cmdBuffer.setViewport(0, { viewport });
			m_cmdBuffer.setScissor(0, { scissor });

			// bind the graphics pipeline
			m_cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

			// draw call (triangle)
			m_cmdBuffer.draw(3u, 1u, 0u, 0u);
		}
		// end render pass
		m_cmdBuffer.endRenderPass();
	}
	m_cmdBuffer.end();

	vk::SubmitInfo submitInfo{};
	submitInfo.setCommandBufferCount(1u).setPCommandBuffers(&m_cmdBuffer);

	Device->graphicsQueue.submit(1u, &submitInfo, {});

	// CHECK:
	Device->waitIdle();
}

void BrdfLutCalculation::EditPods()
{
	PodHandle<EnvironmentMap> envMap = m_envmapAsset->podHandle;

	if (envMap.IsDefault()) {
		return;
	}


	//.. prefiltered and rest here (or other class)
	PodHandle<Image> brdflut = envMap.Lock()->brdfLut;

	if (brdflut.IsDefault()) {
		PodEditor e(envMap);
		auto& [entry, irr] = AssetHandlerManager::CreateEntry<::Image>("gen-data/generated/image");

		e.pod->brdfLut = entry->GetHandleAs<::Image>();
		brdflut = entry->GetHandleAs<::Image>();
	}


	PodHandle<::Image> imgHandle{ brdflut.uid };

	PodEditor imageEditor(imgHandle);

	auto& img = m_attachment;

	img.BlockingTransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

	RBuffer stagingbuffer{ m_resolution * m_resolution * 16u, vk::BufferUsageFlagBits::eTransferDst,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	img.CopyImageToBuffer(stagingbuffer);

	void* data = Device->mapMemory(stagingbuffer.GetMemory(), 0, VK_WHOLE_SIZE, {});

	imageEditor->data.resize(m_resolution * m_resolution * 16u);
	imageEditor->width = m_resolution;
	imageEditor->height = m_resolution;
	imageEditor->format = ImageFormat::Hdr;
	memcpy(imageEditor->data.data(), data, m_resolution * m_resolution * 16u);

	Device->unmapMemory(stagingbuffer.GetMemory());
}

} // namespace vl
