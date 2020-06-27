#include "pch.h"
#include "PrefilteredMapCalculation.h"

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
#include "rendering/assets/GpuCubemap.h"


namespace {
struct PushConstant {
	glm::mat4 rotVp;
	float roughness;
	float skyboxRes;
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
PrefilteredMapCalculation::PrefilteredMapCalculation(EnvironmentMap::Gpu* envmapAsset, uint32 calculationResolution)
	: m_envmapAsset(envmapAsset)
	, m_resolution(calculationResolution)
{
}

void PrefilteredMapCalculation::Calculate()
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

void PrefilteredMapCalculation::MakeDesciptors()
{
	m_skyboxDescLayout.AddBinding(vk::DescriptorType::eCombinedImageSampler, vk::ShaderStageFlagBits::eFragment);
	m_skyboxDescLayout.Generate();

	m_descSet = m_skyboxDescLayout.GetDescriptorSet();

	auto quadSampler = vl::GpuAssetManager->GetDefaultSampler();

	vk::DescriptorImageInfo imageInfo{};
	imageInfo
		.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
		.setImageView(m_envmapAsset->skybox.Lock().cubemap->GetView())
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

void PrefilteredMapCalculation::MakeRenderPass()
{
	vk::AttachmentDescription colorAttachmentDesc{};
	colorAttachmentDesc
		.setFormat(m_envmapAsset->skybox.Lock().cubemap->GetFormat()) // CHECK:
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

void PrefilteredMapCalculation::AllocateCommandBuffers()
{
	vk::CommandBufferAllocateInfo allocInfo{};
	allocInfo.setCommandPool(Device->graphicsCmdPool.get())
		.setLevel(vk::CommandBufferLevel::ePrimary)
		.setCommandBufferCount(6u);

	m_cmdBuffers = Device->allocateCommandBuffers(allocInfo);
}

void PrefilteredMapCalculation::AllocateCubeVertexBuffer()
{
	vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();

	RBuffer vertexStagingbuffer{ vertexBufferSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	// copy data to buffer
	vertexStagingbuffer.UploadData(vertices.data(), vertexBufferSize);


	// device local
	m_cubeVertexBuffer = std::make_unique<RBuffer>(vertexBufferSize,
		vk::BufferUsageFlagBits::eTransferDst | vk::BufferUsageFlagBits::eVertexBuffer,
		vk::MemoryPropertyFlagBits::eDeviceLocal);


	// copy from host to device local
	m_cubeVertexBuffer->CopyBuffer(vertexStagingbuffer);
}

void PrefilteredMapCalculation::MakePipeline()
{
	auto& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/prefiltered.shader");

	if (!gpuShader.HasValidModule()) {
		LOG_ERROR("Geometry Pipeline skipped due to shader compilation errors.");
		return;
	}
	std::vector shaderStages = gpuShader.shaderStages;

	auto& fragShaderModule = gpuShader.frag;
	auto& vertShaderModule = gpuShader.vert;

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
		.setStageFlags(vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment) //
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

void PrefilteredMapCalculation::PrepareFaceInfo()
{
	// create framebuffers for each lod/face
	for (uint32 mip = 0; mip < 6; ++mip) {
		for (uint32 i = 0; i < 6; ++i) {

			// reisze framebuffer according to mip-level size.
			uint32 mipResolution = m_resolution / static_cast<uint32>(std::round((std::pow(2, mip))));

			m_cubemapMips[mip].faceAttachments[i] = std::make_unique<RImageAttachment>("face" + i, mipResolution,
				mipResolution, m_envmapAsset->skybox.Lock().cubemap->GetFormat(), vk::ImageTiling::eOptimal,
				vk::ImageLayout::eUndefined,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eTransferSrc,
				vk::MemoryPropertyFlagBits::eDeviceLocal);
			m_cubemapMips[mip].faceAttachments[i]->BlockingTransitionToLayout(
				vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

			vk::FramebufferCreateInfo createInfo{};
			createInfo
				.setRenderPass(m_renderPass.get()) //
				.setAttachmentCount(1u)
				.setPAttachments(&m_cubemapMips[mip].faceAttachments[i]->GetView())
				.setWidth(mipResolution) // CHECK: parameter
				.setHeight(mipResolution)
				.setLayers(1);

			m_cubemapMips[mip].framebuffers[i] = Device->createFramebufferUnique(createInfo);
		}
	}

	m_captureProjection = glm::perspective(glm::radians(90.0f), 1.0f, 1.0f, 10.0f);
	m_captureProjection[1][1] *= -1;


#pragma warning(disable : 4305)
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

void PrefilteredMapCalculation::RecordAndSubmitCmdBuffers()
{
	Device->waitIdle();

	PROFILE_SCOPE(Renderer);


	// for each mip / framebuffer / face
	for (uint32 mip = 0; mip < 6; ++mip) {
		for (uint32 i = 0; i < 6; ++i) {

			uint32 mipResolution = m_resolution * std::pow(0.5, mip);

			vk::Rect2D scissor{};

			scissor
				.setOffset({ 0, 0 }) //
				.setExtent({ mipResolution, mipResolution });

			vk::Viewport viewport{};

			viewport
				.setWidth(static_cast<float>(mipResolution)) //
				.setHeight(static_cast<float>(mipResolution));


			vk::RenderPassBeginInfo renderPassInfo{};
			renderPassInfo
				.setRenderPass(m_renderPass.get()) //
				.setFramebuffer(m_cubemapMips[mip].framebuffers[i].get());
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


					float roughness = (float)mip / (float)(5 - 1);

					// WIP:
					PushConstant pc{ //
						m_captureProjection * glm::mat4(glm::mat3(m_captureViews[i])), roughness,
						static_cast<float>(m_envmapAsset->skybox.Lock().cubemap->GetExtent2D().width)
					};

					// Submit via push constant (rather than a UBO)
					m_cmdBuffers[i].pushConstants(m_pipelineLayout.get(),
						vk::ShaderStageFlagBits::eVertex | vk::ShaderStageFlagBits::eFragment, 0u, sizeof(PushConstant),
						&pc);

					vk::Buffer vertexBuffers[] = { *m_cubeVertexBuffer };
					vk::DeviceSize offsets[] = { 0 };
					// geom
					m_cmdBuffers[i].bindVertexBuffers(0u, 1u, vertexBuffers, offsets);

					// descriptor sets
					m_cmdBuffers[i].bindDescriptorSets(
						vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u, &m_descSet, 0u, nullptr);

					// draw call (cube)
					m_cmdBuffers[i].draw(static_cast<uint32>(vertices.size() / 3), 1u, 0u, 0u);
				}
				// end render pass
				m_cmdBuffers[i].endRenderPass();
			}
			m_cmdBuffers[i].end();

			vk::SubmitInfo submitInfo{};
			submitInfo.setCommandBufferCount(1u).setPCommandBuffers(&m_cmdBuffers[i]);

			Device->graphicsQueue.submit(1u, &submitInfo, {});
			// CHECK:
			Device->waitIdle();
		}
	}
}

void PrefilteredMapCalculation::EditPods()
{
	PodHandle<EnvironmentMap> envMap = m_envmapAsset->podHandle;

	if (envMap.IsDefault()) {
		return;
	}


	//.. prefiltered and rest here (or other class)
	PodHandle<Cubemap> pref = envMap.Lock()->prefiltered;

	if (pref.IsDefault()) {
		PodEditor e(envMap);
		auto& [entry, irr] = AssetHandlerManager::CreateEntry<Cubemap>("generated/cubemap");

		e.pod->prefiltered = entry->GetHandleAs<Cubemap>();
		pref = entry->GetHandleAs<Cubemap>();
	}

	PodHandle<Cubemap> cubemapHandle{ pref.uid };

	PodEditor cubemapEditor(cubemapHandle);

	cubemapEditor->resolution = m_resolution;
	cubemapEditor->format = m_envmapAsset->skybox.Lock().podHandle.Lock()->format;


	size_t bufferSize{ 0llu };
	auto bytesPerPixel = cubemapEditor->format == ImageFormat::Hdr ? 4u * 4u : 4u;
	for (uint32 mip = 0; mip < 6; ++mip) {

		auto res = m_resolution / std::pow(2, mip);
		bufferSize += res * res * bytesPerPixel * 6;
	}

	cubemapEditor->data.resize(bufferSize);
	cubemapEditor->mipCount = 6u;

	size_t offset{ 0llu };
	for (uint32 mip = 0; mip < 6; ++mip) {
		for (uint32 i = 0; i < 6; ++i) {

			size_t res = m_resolution / std::pow(2, mip);
			size_t size = res * res * bytesPerPixel;

			auto& img = m_cubemapMips[mip].faceAttachments[i];

			img->BlockingTransitionToLayout(
				vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::eTransferSrcOptimal);

			vl::RBuffer stagingbuffer{ vk::DeviceSize(size), vk::BufferUsageFlagBits::eTransferDst,
				vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

			img->CopyImageToBuffer(stagingbuffer);

			void* data = Device->mapMemory(stagingbuffer.GetMemory(), 0, VK_WHOLE_SIZE, {});

			memcpy(cubemapEditor->data.data() + offset, data, size);

			Device->unmapMemory(stagingbuffer.GetMemory());

			offset += size;
		}
	}
}

} // namespace vl
