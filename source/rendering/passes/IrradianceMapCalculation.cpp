#include "pch.h"
#include "IrradianceMapCalculation.h"

#include "assets/PodEditor.h"
#include "engine/Engine.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Layouts.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"

namespace {
struct PushConstant {
	glm::mat4 v;
	glm::mat4 p;
};

static_assert(sizeof(PushConstant) <= 128);


std::array vertices = {
	// back face
	-1.0f, -1.0f, -1.0f, // bottom-left
	1.0f, 1.0f, -1.0f,   // top-right
	1.0f, -1.0f, -1.0f,  // bottom-right
	1.0f, 1.0f, -1.0f,   // top-right
	-1.0f, -1.0f, -1.0f, // bottom-left
	-1.0f, 1.0f, -1.0f,  // top-left
	// front face
	-1.0f, -1.0f, 1.0f, // bottom-left
	1.0f, -1.0f, 1.0f,  // bottom-right
	1.0f, 1.0f, 1.0f,   // top-right
	1.0f, 1.0f, 1.0f,   // top-right
	-1.0f, 1.0f, 1.0f,  // top-left
	-1.0f, -1.0f, 1.0f, // bottom-left
	// left face
	-1.0f, 1.0f, 1.0f,   // top-right
	-1.0f, 1.0f, -1.0f,  // top-left
	-1.0f, -1.0f, -1.0f, // bottom-left
	-1.0f, -1.0f, -1.0f, // bottom-left
	-1.0f, -1.0f, 1.0f,  // bottom-right
	-1.0f, 1.0f, 1.0f,   // top-right
	// right face
	1.0f, 1.0f, 1.0f,   // top-left
	1.0f, -1.0f, -1.0f, // bottom-right
	1.0f, 1.0f, -1.0f,  // top-right
	1.0f, -1.0f, -1.0f, // bottom-right
	1.0f, 1.0f, 1.0f,   // top-left
	1.0f, -1.0f, 1.0f,  // bottom-left
	// bottom face
	-1.0f, -1.0f, -1.0f, // top-right
	1.0f, -1.0f, -1.0f,  // top-left
	1.0f, -1.0f, 1.0f,   // bottom-left
	1.0f, -1.0f, 1.0f,   // bottom-left
	-1.0f, -1.0f, 1.0f,  // bottom-right
	-1.0f, -1.0f, -1.0f, // top-right
	// top face
	-1.0f, 1.0f, -1.0f, // top-left
	1.0f, 1.0f, 1.0f,   // bottom-right
	1.0f, 1.0f, -1.0f,  // top-right
	1.0f, 1.0f, 1.0f,   // bottom-right
	-1.0f, 1.0f, -1.0f, // top-left
	-1.0f, 1.0f, 1.0f,  // bottom-left
};
} // namespace


namespace vl {
IrradianceMapCalculation::IrradianceMapCalculation(::Cubemap::Gpu* cubemapAsset, uint32 calculationResolution)
	: m_cubemapAsset(cubemapAsset)
	, m_resolution(calculationResolution)
{
}

void IrradianceMapCalculation::Calculate()
{
	MakeDesciptors();

	MakeRenderPass();

	AllocateCommandBuffers();

	AllocateCubeVertexBuffer();

	MakePipeline();

	PrepareFaceInfo();

	RecordAndSubmitCmdBuffers();

	EditPods();
}

void IrradianceMapCalculation::MakeDesciptors()
{
	m_skyboxDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	m_skyboxDescLayout.Generate();

	m_descSet = m_skyboxDescLayout.GetDescriptorSet();


	auto quadSampler = vl::GpuAssetManager->GetDefaultSampler();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(m_cubemapAsset->cubemap->GetView())
		.setSampler(quadSampler);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(m_descSet) //
		.setDstBinding(0u)    // 0 is for the Ubo
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setDescriptorCount(1u)
		.setPBufferInfo(nullptr)
		.setPImageInfo(&imageInfo)
		.setPTexelBufferView(nullptr);

	vl::Device->updateDescriptorSets(descriptorWrite, {});
}

void IrradianceMapCalculation::MakeRenderPass()
{
	vk::AttachmentDescription colorAttachmentDesc{};
	colorAttachmentDesc
		.setFormat(m_cubemapAsset->cubemap->GetFormat()) // CHECK:
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eColorAttachmentOptimal) // CHECK: vk::ImageLayout::eShaderReadOnlyOptimal?
		.setFinalLayout(vk::ImageLayout::eColorAttachmentOptimal);  // CHECK:

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

void IrradianceMapCalculation::AllocateCommandBuffers()
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(6u);

	m_cmdBuffers = Device->allocateCommandBuffers(allocInfo);
}

void IrradianceMapCalculation::AllocateCubeVertexBuffer()
{
	vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();

	RawBuffer vertexStagingBuffer{ vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	vertexStagingBuffer.UploadData(vertices.data(), vertexBufferSize);


	// device local
	m_cubeVertexBuffer = std::make_unique<RawBuffer>(vertexBufferSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal);


	// copy from host to device local
	m_cubeVertexBuffer->CopyBuffer(vertexStagingBuffer);
}

void IrradianceMapCalculation::MakePipeline()
{
	auto& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/irradiance.vert");

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
		.setStride(sizeof(float) * 3)
		.setInputRate(vk::VertexInputRate::eVertex);

	vk::VertexInputAttributeDescription attributeDescription{};

	attributeDescription
		.setBinding(0u) //
		.setLocation(0u)
		.setFormat(vk::Format::eR32G32B32Sfloat)
		.setOffset(0u);

	vertexInputInfo
		.setVertexBindingDescriptionCount(1u) //
		.setVertexAttributeDescriptionCount(1u)
		.setPVertexBindingDescriptions(&bindingDescription)
		.setPVertexAttributeDescriptions(&attributeDescription);


	vk::PipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo
		.setStage(vk::ShaderStageFlagBits::eFragment) //
		.setModule(*fragShaderModule)
		.setPName("main");

	std::array shaderStages = { vertShaderStageInfo, fragShaderStageInfo };

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

	static ConsoleVariable<uint> fillmode{ "fillmode", 0 };

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer
		.setDepthClampEnable(VK_FALSE) //
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(static_cast<vk::PolygonMode>(fillmode.Get()))
		.setLineWidth(1.f)
		.setCullMode(vk::CullModeFlagBits::eNone)
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


	// pipeline layout
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);


	std::array layouts = { m_skyboxDescLayout.setLayout.get() };

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

void IrradianceMapCalculation::PrepareFaceInfo()
{
	// create framebuffers for each face
	for (uint32 i = 0; i < 6; ++i) {

		m_faceAttachments[i] = std::make_unique<ImageAttachment>("face" + i, m_resolution, m_resolution,
			m_cubemapAsset->cubemap->GetFormat(), vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eDeviceLocal, false);
		m_faceAttachments[i]->BlockingTransitionToLayout(
			vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

		vk::FramebufferCreateInfo createInfo{};
		createInfo
			.setRenderPass(m_renderPass.get()) //
			.setAttachmentCount(1u)
			.setPAttachments(&m_faceAttachments[i]->GetView())
			.setWidth(m_resolution) // CHECK: parameter
			.setHeight(m_resolution)
			.setLayers(1);

		m_framebuffers[i] = Device->createFramebufferUnique(createInfo);
	}

	m_captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 10.0f);
	static ConsoleVariable<int> m{ "invProj", -1 };
	m_captureProjection[1][1] *= m;

	m_captureViews

		= {
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),  //+x
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)), //-x
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)), //+y
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)), // -y
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),  // +z
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)), // -z
		  };
}

void IrradianceMapCalculation::RecordAndSubmitCmdBuffers()
{
	Device->waitIdle();

	PROFILE_SCOPE(Renderer);

	vk::Rect2D scissor{};

	scissor
		.setOffset({ 0, 0 }) //
		.setExtent({ m_resolution, m_resolution });

	vk::Viewport viewport{};

	viewport
		.setWidth(m_resolution) //
		.setHeight(m_resolution);


	// for each framebuffer / face
	for (uint32 i = 0; i < 6; ++i) {

		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo
			.setRenderPass(m_renderPass.get()) //
			.setFramebuffer(m_framebuffers[i].get());
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
		m_cmdBuffers[i].begin(beginInfo);
		{

			// PERF: needs render pass?
			// begin render pass
			m_cmdBuffers[i].beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
			{
				// Dynamic viewport & scissor
				m_cmdBuffers[i].setViewport(0, { viewport });
				m_cmdBuffers[i].setScissor(0, { scissor });

				// bind the graphics pipeline
				m_cmdBuffers[i].bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());


				PushConstant pc{ //
					m_captureViews[i], m_captureProjection
				};

				// Submit via push constant (rather than a UBO)
				m_cmdBuffers[i].pushConstants(
					m_pipelineLayout.get(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

				vk::Buffer vertexBuffers[] = { *m_cubeVertexBuffer };
				vk::DeviceSize offsets[] = { 0 };
				// geom
				m_cmdBuffers[i].bindVertexBuffers(0u, 1u, vertexBuffers, offsets);

				// descriptor sets
				m_cmdBuffers[i].bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u, &m_descSet, 0u, nullptr);

				// draw call (cube)
				m_cmdBuffers[i].draw(vertices.size() / 3, 1u, 0u, 0u);
			}
			// end render pass
			m_cmdBuffers[i].endRenderPass();
		}
		m_cmdBuffers[i].end();

		vk::SubmitInfo submitInfo{};
		submitInfo.setCommandBufferCount(1u).setPCommandBuffers(&m_cmdBuffers[i]);

		Device->graphicsQueue.submit(1u, &submitInfo, {});
	}

	// CHECK:
	Device->waitIdle();
}

void IrradianceMapCalculation::EditPods()
{
	PodHandle<::Cubemap> cubemapHandle{ m_cubemapAsset->podUid };
	PodEditor editor{ cubemapHandle };

	auto pod = editor.GetEditablePtr();

	for (uint32 i = 0; i < 6; ++i) {

		auto& img = m_faceAttachments[i];

		img->BlockingTransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

		auto bytesPerPixel = pod->format == ImageFormat::Hdr ? 4u * 4u : 4u;
		vl::RawBuffer stagingBuffer{ m_resolution * m_resolution * bytesPerPixel, vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		img->CopyImageToBuffer(stagingBuffer);

		void* data = Device->mapMemory(stagingBuffer.GetMemory(), 0, VK_WHOLE_SIZE, {});

		PodEditor faceEditor{ pod->irradiance[i] };

		auto facePod = faceEditor.GetEditablePtr();

		facePod->data.resize(m_resolution * m_resolution * bytesPerPixel);
		facePod->width = m_resolution;
		facePod->height = m_resolution;
		memcpy(facePod->data.data(), data, m_resolution * m_resolution * bytesPerPixel);

		Device->unmapMemory(stagingBuffer.GetMemory());
	}
}

} // namespace vl