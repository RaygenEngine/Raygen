#include "pch.h"
#include "GeometryPass.h"

#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/renderer/Renderer.h"
#include "rendering/assets/GpuMesh.h"

#include <glm/gtc/matrix_inverse.hpp>

namespace vl {

void GeometryPass::InitAll()
{
	InitRenderPass();

	m_materialDescLayout.AddBinding(vk::DescriptorType::eUniformBuffer, vk::ShaderStageFlagBits::eFragment);

	for (uint32 i = 0; i < 5u; ++i) {
		m_materialDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	}

	m_materialDescLayout.Generate();


	MakePipeline();
}

void GeometryPass::InitRenderPass()
{
	std::array<vk::AttachmentDescription, 5> colorAttachmentDescs{};
	std::array<vk::AttachmentReference, 5> colorAttachmentRefs{};

	for (size_t i = 0; i < 5; ++i) {
		colorAttachmentDescs[i]
			.setFormat(GBuffer::colorAttachmentFormats[i]) // CHECK:
			.setSamples(vk::SampleCountFlagBits::e1)
			.setLoadOp(vk::AttachmentLoadOp::eClear)
			.setStoreOp(vk::AttachmentStoreOp::eStore)
			.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
			.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
			.setInitialLayout(
				vk::ImageLayout::eColorAttachmentOptimal)             // CHECK: vk::ImageLayout::eShaderReadOnlyOptimal?
			.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // CHECK:

		colorAttachmentRefs[i]
			.setAttachment(static_cast<uint32>(i)) //
			.setLayout(vk::ImageLayout::eColorAttachmentOptimal);
	}

	vk::AttachmentDescription depthAttachmentDesc{};
	depthAttachmentDesc
		.setFormat(Device->pd->FindDepthFormat()) // CHECK:
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare) // CHECK: if use stencil dont forget those two
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(
			vk::ImageLayout::eDepthStencilAttachmentOptimal)      // CHECK: vk::ImageLayout::eShaderReadOnlyOptimal?
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal); // CHECK:

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef
		.setAttachment(5u) //
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::SubpassDescription subpass{};
	subpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(static_cast<uint32>(colorAttachmentRefs.size()))
		.setPColorAttachments(colorAttachmentRefs.data())
		.setPDepthStencilAttachment(&depthAttachmentRef);

	vk::SubpassDependency dependency{};
	dependency
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlags(0)) // 0
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite);

	std::array attachments{ colorAttachmentDescs[0], colorAttachmentDescs[1], colorAttachmentDescs[2],
		colorAttachmentDescs[3], colorAttachmentDescs[4], depthAttachmentDesc };
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

void GeometryPass::InitFramebuffers()
{
	vk::Extent2D fbSize = Renderer->m_viewportFramebufferSize;

	m_gBuffer = std::make_unique<GBuffer>(fbSize.width, fbSize.height);

	// framebuffers

	auto imAttachments = m_gBuffer->GetViewsArray();

	vk::FramebufferCreateInfo createInfo{};
	createInfo
		.setRenderPass(m_renderPass.get()) //
		.setAttachmentCount(static_cast<uint32>(imAttachments.size()))
		.setPAttachments(imAttachments.data())
		.setWidth(fbSize.width)
		.setHeight(fbSize.height)
		.setLayers(1);

	m_framebuffer = Device->createFramebufferUnique(createInfo);
}

void GeometryPass::MakePipeline()
{
	static auto& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/gbuffer.vert");
	gpuShader.onCompile = [&]() {
		Device->waitIdle();
		MakePipeline();
	};

	if (!gpuShader.HasCompiledSuccessfully()) {
		LOG_ERROR("Geometry Pipeline skipped due to shader compilation errors.");
		return;
	}
	auto& fragShaderModule = gpuShader.frag;
	auto& vertShaderModule = gpuShader.vert;

	vk::PipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eVertex) //
		.setModule(*vertShaderModule)
		.setPName("main");
	// fixed-function stage
	vk::PipelineVertexInputStateCreateInfo vertexInputInfo{};

	vk::VertexInputBindingDescription bindingDescription{};
	bindingDescription
		.setBinding(0u) //
		.setStride(sizeof(Vertex))
		.setInputRate(vk::VertexInputRate::eVertex);

	std::array<vk::VertexInputAttributeDescription, 5> attributeDescriptions{};

	attributeDescriptions[0].binding = 0u;
	attributeDescriptions[0].location = 0u;
	attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[0].offset = offsetof(Vertex, position);

	attributeDescriptions[1].binding = 0u;
	attributeDescriptions[1].location = 1u;
	attributeDescriptions[1].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[1].offset = offsetof(Vertex, normal);

	attributeDescriptions[2].binding = 0u;
	attributeDescriptions[2].location = 2u;
	attributeDescriptions[2].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[2].offset = offsetof(Vertex, tangent);

	attributeDescriptions[3].binding = 0u;
	attributeDescriptions[3].location = 3u;
	attributeDescriptions[3].format = vk::Format::eR32G32B32Sfloat;
	attributeDescriptions[3].offset = offsetof(Vertex, bitangent);

	attributeDescriptions[4].binding = 0u;
	attributeDescriptions[4].location = 4u;
	attributeDescriptions[4].format = vk::Format::eR32G32Sfloat;
	attributeDescriptions[4].offset = offsetof(Vertex, uv);

	vertexInputInfo
		.setVertexBindingDescriptionCount(1u) //
		.setVertexAttributeDescriptionCount(static_cast<uint32_t>(attributeDescriptions.size()))
		.setPVertexBindingDescriptions(&bindingDescription)
		.setPVertexAttributeDescriptions(attributeDescriptions.data());


	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eFragment) //
		.setModule(*fragShaderModule)
		.setPName("main");

	vk::PipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };


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

	std::array<vk::PipelineColorBlendAttachmentState, 5> colorBlendAttachment{};
	for (uint32 i = 0u; i < 5; ++i) {
		colorBlendAttachment[i]
			.setColorWriteMask(vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
							   | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA) //
			.setBlendEnable(VK_FALSE)
			.setSrcColorBlendFactor(vk::BlendFactor::eOne)
			.setDstColorBlendFactor(vk::BlendFactor::eZero)
			.setColorBlendOp(vk::BlendOp::eAdd)
			.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
			.setDstAlphaBlendFactor(vk::BlendFactor::eZero)
			.setAlphaBlendOp(vk::BlendOp::eAdd);
	}


	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending
		.setLogicOpEnable(VK_FALSE) //
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachmentCount(static_cast<uint32>(colorBlendAttachment.size()))
		.setPAttachments(colorBlendAttachment.data())
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
		.setSize(sizeof(glm::mat4) * 2)
		.setOffset(0u);


	std::array layouts = { m_materialDescLayout.setLayout.get(), Renderer->GetCameraDescLayout() };

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
		.setPSetLayouts(layouts.data())
		.setPushConstantRangeCount(1u)
		.setPPushConstantRanges(&pushConstantRange);

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);

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

	m_pipeline = Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

namespace {
	struct PushConstant {
		glm::mat4 modelMat;
		glm::mat4 normalMat;
	};

	static_assert(sizeof(PushConstant) <= 128);
} // namespace

void GeometryPass::RecordGeometryDraw(vk::CommandBuffer* cmdBuffer)
{
	PROFILE_SCOPE(Renderer);


	if (Input.IsJustPressed(Key::Comma)) {
		Device->waitIdle();
		MakePipeline();
		LOG_REPORT("Remade pipeline");
		Device->waitIdle();
	}

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
			.setExtent(Renderer->m_viewportRect.extent);

		std::array<vk::ClearValue, 6> clearValues = {};
		clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[1].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[2].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[3].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[4].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
		clearValues[5].setDepthStencil({ 1.0f, 0 });
		renderPassInfo
			.setClearValueCount(static_cast<uint32>(clearValues.size())) //
			.setPClearValues(clearValues.data());

		// PERF: needs render pass?
		// begin render pass
		cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
		{
			auto camera = Scene->GetActiveCamera();
			if (!camera) {
				cmdBuffer->endRenderPass();
				cmdBuffer->end();
				return;
			}

			// descriptor sets
			cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 1u, 1u,
				&Renderer->GetCameraDescSet(), 0u, nullptr);

			// Dynamic viewport & scissor
			cmdBuffer->setViewport(0, { GetViewport() });
			cmdBuffer->setScissor(0, { GetScissor() });

			// bind the graphics pipeline
			cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());

			for (auto model : Scene->geometries.elements) {
				if (!model) {
					continue;
				}

				glm::mat4 normalMat = glm::inverseTranspose(glm::mat3(model->transform));
				PushConstant pc{ //
					model->transform, normalMat
				};

				// Submit via push constant (rather than a UBO)
				cmdBuffer->pushConstants(
					m_pipelineLayout.get(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

				for (auto& gg : model->model.Lock().geometryGroups) {
					vk::Buffer vertexBuffers[] = { *gg.vertexBuffer };
					vk::DeviceSize offsets[] = { 0 };
					// geom
					cmdBuffer->bindVertexBuffers(0u, 1u, vertexBuffers, offsets);

					// indices
					cmdBuffer->bindIndexBuffer(*gg.indexBuffer, 0, vk::IndexType::eUint32);

					// descriptor sets
					cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u,
						&gg.material.Lock().descriptorSet, 0u, nullptr);

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

vk::DescriptorSet GeometryPass::GetMaterialDescriptorSet() const
{
	return m_materialDescLayout.GetDescriptorSet();
}

vk::Viewport GeometryPass::GetViewport() const
{
	auto vpSize = Renderer->m_viewportRect.extent;

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
		.setExtent(Renderer->m_viewportRect.extent);

	return scissor;
}
} // namespace vl
