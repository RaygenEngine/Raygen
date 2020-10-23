#include "IrradianceMapCalculation.h"

#include "assets/AssetRegistry.h"
#include "assets/PodEditor.h"
#include "assets/pods/Cubemap.h"
#include "assets/pods/EnvironmentMap.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuCubemap.h"
#include "rendering/assets/GpuEnvironmentMap.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Renderer.h"
#include "rendering/scene/Scene.h"
#include "rendering/wrappers/CmdBuffer.h"

namespace {
struct PushConstant {
	glm::mat4 rotVp;
};

static_assert(sizeof(PushConstant) <= 128);


std::array vertices = {
	// positions
	-1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f,
	-1.0f,

	-1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f,
	1.0f,

	1.0f, -1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f, -1.0f, 1.0f,

	-1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f, -1.0f, 1.0f, 1.0f, -1.0f, 1.0f, -1.0f,

	-1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f, -1.0f, 1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f, 1.0f, -1.0f,
	1.0f
};
} // namespace


namespace vl {
IrradianceMapCalculation::IrradianceMapCalculation(GpuEnvironmentMap* envmapAsset, uint32 calculationResolution)
	: m_envmapAsset(envmapAsset)
	, m_resolution(calculationResolution)
{
}

void IrradianceMapCalculation::Calculate()
{
	MakeDesciptors();

	MakeRenderPass();

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

	m_descSet = m_skyboxDescLayout.AllocDescriptorSet();

	auto quadSampler = GpuAssetManager->GetDefaultSampler();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(m_envmapAsset->skybox.Lock().cubemap.view())
		.setSampler(quadSampler);

	vk::WriteDescriptorSet descriptorWrite{};
	descriptorWrite
		.setDstSet(m_descSet) //
		.setDstBinding(0u)    // 0 is for the Ubo
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
		.setImageInfo(imageInfo);

	Device->updateDescriptorSets(descriptorWrite, {});
}

void IrradianceMapCalculation::MakeRenderPass()
{
	vk::AttachmentDescription colorAttachmentDesc{};
	colorAttachmentDesc
		.setFormat(m_envmapAsset->skybox.Lock().cubemap.format) //
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

void IrradianceMapCalculation::AllocateCubeVertexBuffer()
{
	vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();

	RBuffer vertexStagingbuffer{ vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	vertexStagingbuffer.UploadData(vertices.data(), vertexBufferSize);


	// device local
	m_cubeVertexBuffer
		= RBuffer{ vertexBufferSize, vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
			  vk::MemoryPropertyFlagBits::eDeviceLocal };


	// copy from host to device local
	m_cubeVertexBuffer.CopyBuffer(vertexStagingbuffer);
}

void IrradianceMapCalculation::MakePipeline()
{
	auto& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/offline/irradiance.shader");

	if (!gpuShader.HasValidModule()) {
		LOG_ERROR("Geometry Pipeline skipped due to shader compilation errors.");
		return;
	}
	std::vector shaderStages = gpuShader.shaderStages;

	auto& fragShaderModule = gpuShader.frag;
	auto& vertShaderModule = gpuShader.vert;


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
		.setVertexBindingDescriptions(bindingDescription) //
		.setVertexAttributeDescriptions(attributeDescription);

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

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer
		.setDepthClampEnable(VK_FALSE) //
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(static_cast<vk::PolygonMode>(vk::PolygonMode::eFill))
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
		.setAttachments(colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });

	// Dynamic vieport
	std::array dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };
	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.setDynamicStates(dynamicStates);

	// pipeline layout
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eVertex) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);

	std::array layouts{ m_skyboxDescLayout.handle() };
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayouts(layouts) //
		.setPushConstantRanges(pushConstantRange);

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

		m_faceAttachments[i] = RImageAttachment{ m_resolution, m_resolution,
			m_envmapAsset->skybox.Lock().cubemap.format, vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
			vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
			vk::MemoryPropertyFlagBits::eDeviceLocal, "face" + i };
		m_faceAttachments[i].BlockingTransitionToLayout(
			vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

		std::array attachments{ m_faceAttachments[i].view() };

		vk::FramebufferCreateInfo createInfo{};
		createInfo
			.setRenderPass(m_renderPass.get()) //
			.setAttachments(attachments)
			.setWidth(m_resolution)
			.setHeight(m_resolution)
			.setLayers(1);

		m_framebuffer[i] = Device->createFramebufferUnique(createInfo);
	}

	m_captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 10.0f);
	m_captureProjection[1][1] *= -1;


	m_captureViews

		= {
			  // right
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
			  // left
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
			  // up
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),
			  // down
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f)),
			  // front
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
			  // back
			  glm::lookAt(glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f, 1.0f, 0.0f)),
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
		.setWidth(static_cast<float>(m_resolution)) //
		.setHeight(static_cast<float>(m_resolution));


	// for each framebuffer / face
	for (uint32 i = 0; i < 6; ++i) {

		CmdBuffer<Graphics> cmdBuffer{ vk::CommandBufferLevel::ePrimary };

		vk::RenderPassBeginInfo renderPassInfo{};
		renderPassInfo
			.setRenderPass(m_renderPass.get()) //
			.setFramebuffer(m_framebuffer[i].get());
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


				PushConstant pc{ //
					m_captureProjection * glm::mat4(glm::mat3(m_captureViews[i]))
				};

				// Submit via push constant (rather than a UBO)
				cmdBuffer.pushConstants(
					m_pipelineLayout.get(), vk::ShaderStageFlagBits::eVertex, 0u, sizeof(PushConstant), &pc);

				// geom
				cmdBuffer.bindVertexBuffers(0u, { m_cubeVertexBuffer.handle() }, { vk::DeviceSize(0) });

				// descriptor sets
				cmdBuffer.bindDescriptorSets(
					vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, { m_descSet }, nullptr);

				// draw call (cube)
				cmdBuffer.draw(static_cast<uint32>(vertices.size() / 3), 1u, 0u, 0u);
			}
			// end render pass
			cmdBuffer.endRenderPass();
		}
		cmdBuffer.end();

		cmdBuffer.submit();

		Device->waitIdle();
	}

	Device->waitIdle();
}

void IrradianceMapCalculation::EditPods()
{
	PodHandle<EnvironmentMap> envMap = m_envmapAsset->podHandle;

	if (envMap.IsDefault()) {
		return;
	}


	//.. prefiltered and rest here (or other class)
	PodHandle<Cubemap> irradiance = envMap.Lock()->irradiance;

	if (irradiance.IsDefault()) {
		PodEditor e(envMap);
		auto& [entry, irr] = AssetRegistry::CreateEntry<Cubemap>("gen-data/generated/cubemap");

		e.pod->irradiance = entry->GetHandleAs<Cubemap>();
		irradiance = entry->GetHandleAs<Cubemap>();
	}


	PodEditor cubemapEditor(irradiance);

	cubemapEditor->resolution = m_resolution;
	cubemapEditor->format = m_envmapAsset->skybox.Lock().podHandle.Lock()->format;

	auto bytesPerPixel = cubemapEditor->format == ImageFormat::Hdr ? 4u * 4u : 4u;
	auto size = m_resolution * m_resolution * bytesPerPixel * 6;

	cubemapEditor->data.resize(size);


	for (uint32 i = 0; i < 6; ++i) {

		auto& img = m_faceAttachments[i];

		img.BlockingTransitionToLayout(vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);


		RBuffer stagingbuffer{ m_resolution * m_resolution * bytesPerPixel, vk::BufferUsageFlagBits::eTransferDst,
			vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

		img.CopyImageToBuffer(stagingbuffer);

		void* data = Device->mapMemory(stagingbuffer.memory(), 0, VK_WHOLE_SIZE, {});

		memcpy(cubemapEditor->data.data() + size * i / 6, data, m_resolution * m_resolution * bytesPerPixel);

		Device->unmapMemory(stagingbuffer.memory());
	}
}

} // namespace vl
