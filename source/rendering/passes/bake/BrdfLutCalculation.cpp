#include "BrdfLutCalculation.h"

#include "engine/console/ConsoleVariable.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/Device.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/wrappers/CmdBuffer.h"

namespace vl {
BrdfLutCalculation::BrdfLutCalculation(GpuEnvironmentMap* envmapAsset, uint32 calculationResolution)
	: m_envmapAsset(envmapAsset)
	, m_resolution(calculationResolution)
{
}

void BrdfLutCalculation::Calculate()
{
	MakeRenderPass();

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
		.setColorAttachments(colorAttachmentRef);

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
		.setAttachments(attachments) //
		.setSubpasses(subpass)
		.setDependencies(dependency);

	m_renderPass = Device->createRenderPassUnique(renderPassInfo);
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
		.setViewports(viewport) //
		.setScissors(scissor);

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
		.setAttachments(colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

	// Dynamic vieport
	std::array dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.setDynamicStates(dynamicStates);

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};

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
		.setStages(shaderStages) //
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
	// WIP: remove this class
	// m_attachment = RImage2D{ m_resolution, m_resolution, vk::Format::eR32G32B32A32Sfloat, vk::ImageTiling::eOptimal,
	//	vk::ImageLayout::eUndefined, vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
	//	vk::MemoryPropertyFlagBits::eDeviceLocal, "face" };
	// m_attachment.BlockingTransitionToLayout(vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

	std::array attachments{ m_attachment.view() };

	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(m_renderPass.get()) //
		.setAttachments(attachments)
		.setWidth(m_resolution)
		.setHeight(m_resolution)
		.setLayers(1);

	m_framebuffer = Device->createFramebufferUnique(createInfo);
}

void BrdfLutCalculation::RecordAndSubmitCmdBuffers()
{
	Device->waitIdle();

	CmdBuffer<Graphics> cmdBuffer{ vk::CommandBufferLevel::ePrimary };

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
	renderPassInfo.setClearValues(clearValues);

	cmdBuffer.begin();
	{
		// begin render pass
		cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			// Dynamic viewport & scissor
			cmdBuffer.setViewport(0, { viewport });
			cmdBuffer.setScissor(0, { scissor });

			// bind the graphics pipeline
			cmdBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

			// draw call (triangle)
			cmdBuffer.draw(3u, 1u, 0u, 0u);
		}
		// end render pass
		cmdBuffer.endRenderPass();
	}
	cmdBuffer.end();

	cmdBuffer.submit();

	// CHECK:
	Device->waitIdle();
}

void BrdfLutCalculation::EditPods()
{
	// PodHandle<EnvironmentMap> envMap = m_envmapAsset->podHandle;

	// if (envMap.IsDefault()) {
	//	return;
	//}


	////.. prefiltered and rest here (or other class)
	// PodHandle<Image> brdflut = envMap.Lock()->brdfLut;

	// if (brdflut.IsDefault()) {
	//	PodEditor e(envMap);
	//	auto& [entry, irr] = AssetRegistry::CreateEntry<::Image>("gen-data/generated/image");

	//	e.pod->brdfLut = entry->GetHandleAs<::Image>();
	//	brdflut = entry->GetHandleAs<::Image>();
	//}


	// PodHandle<::Image> imgHandle{ brdflut.uid };

	// PodEditor imageEditor(imgHandle);

	// auto& img = m_attachment;

	// img.BlockingTransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

	// RBuffer stagingbuffer{ m_resolution * m_resolution * 16u, vk::BufferUsageFlagBits::eTransferDst,
	//	vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// img.CopyImageToBuffer(stagingbuffer);

	// void* data = Device->mapMemory(stagingbuffer.memory(), 0, VK_WHOLE_SIZE, {});

	// imageEditor->data.resize(m_resolution * m_resolution * 16u);
	// imageEditor->width = m_resolution;
	// imageEditor->height = m_resolution;
	// imageEditor->format = ImageFormat::Hdr;
	// memcpy(imageEditor->data.data(), data, m_resolution * m_resolution * 16u);

	// Device->unmapMemory(stagingbuffer.memory());
}

} // namespace vl
