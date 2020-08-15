#include "pch.h"
#include "Renderer.h"

#include "rendering/Renderer.h"

#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/passes/DepthmapPass.h"
#include "rendering/passes/GBufferPass.h"
#include "rendering/passes/UnlitPass.h"
#include "rendering/ppt/techniques/PtDebug.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneDirectionalLight.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/structures/Depthmap.h"
#include "rendering/VulkanUtl.h"
#include "rendering/wrappers/Swapchain.h"

#include <editor/imgui/ImguiImpl.h>

namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

Renderer_::Renderer_()
{
	Event::OnViewportUpdated.BindFlag(this, m_didViewportResize);

	vk::AttachmentDescription depthAttachmentDesc{};
	depthAttachmentDesc
		.setFormat(Device->FindDepthFormat()) //
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eLoad)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
		.setFinalLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentReference depthAttachmentRef{};
	depthAttachmentRef
		.setAttachment(2u) //
		.setLayout(vk::ImageLayout::eDepthStencilAttachmentOptimal);

	vk::AttachmentDescription colorAttachmentDesc{};
	vk::AttachmentReference colorAttachmentRef{};

	colorAttachmentDesc.setFormat(vk::Format::eR32G32B32A32Sfloat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	colorAttachmentRef
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);

	vk::AttachmentReference colorAttachmentRef15{};

	colorAttachmentRef15
		.setAttachment(0u) //
		.setLayout(vk::ImageLayout::eShaderReadOnlyOptimal);


	vk::AttachmentDescription colorAttachmentDesc2{};
	vk::AttachmentReference colorAttachmentRef2{};

	colorAttachmentDesc2.setFormat(vk::Format::eR32G32B32A32Sfloat)
		.setSamples(vk::SampleCountFlagBits::e1)
		.setLoadOp(vk::AttachmentLoadOp::eClear)
		.setStoreOp(vk::AttachmentStoreOp::eStore)
		.setStencilLoadOp(vk::AttachmentLoadOp::eDontCare)
		.setStencilStoreOp(vk::AttachmentStoreOp::eDontCare)
		.setInitialLayout(vk::ImageLayout::eUndefined)
		.setFinalLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

	colorAttachmentRef2
		.setAttachment(1u) //
		.setLayout(vk::ImageLayout::eColorAttachmentOptimal);


	vk::SubpassDescription lightSubpass{};
	lightSubpass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(1u)
		.setPColorAttachments(&colorAttachmentRef)
		.setPDepthStencilAttachment(nullptr);

	vk::SubpassDependency lightDep{};
	lightDep
		.setSrcSubpass(VK_SUBPASS_EXTERNAL) //
		.setDstSubpass(0u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eBottomOfPipe)
		.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setSrcAccessMask(vk::AccessFlagBits::eMemoryRead)
		.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion);


	vk::SubpassDescription debugSupass{};
	debugSupass
		.setPipelineBindPoint(vk::PipelineBindPoint::eGraphics) //
		.setColorAttachmentCount(1u)
		.setInputAttachmentCount(1u)
		.setPDepthStencilAttachment(&depthAttachmentRef)
		.setPInputAttachments(&colorAttachmentRef15)
		.setPColorAttachments(&colorAttachmentRef2);

	vk::SubpassDependency debugDep{};
	debugDep
		.setSrcSubpass(0u) //
		.setDstSubpass(1u)
		.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput)
		.setDstStageMask(vk::PipelineStageFlagBits::eFragmentShader)
		.setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
		.setDstAccessMask(vk::AccessFlagBits::eShaderRead)
		.setDependencyFlags(vk::DependencyFlagBits::eByRegion);

	std::array subpasses{ lightSubpass, debugSupass };
	std::array dependcies{ lightDep, debugDep };
	std::array attachments{ colorAttachmentDesc, colorAttachmentDesc2, depthAttachmentDesc };


	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachmentCount(static_cast<uint32>(attachments.size())) //
		.setPAttachments(attachments.data())
		.setSubpassCount(static_cast<uint32>(subpasses.size()))
		.setPSubpasses(subpasses.data())
		.setDependencyCount(static_cast<uint32>(dependcies.size()))
		.setPDependencies(dependcies.data());

	m_ptRenderpass = Device->createRenderPassUnique(renderPassInfo);

	// descsets
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_ppDescSet[i] = Layouts->singleSamplerDescLayout.GetDescriptorSet();
	}

	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_rtDescSet[i] = Layouts->rtTriangleGeometry.GetDescriptorSet();
	}
}

void Renderer_::InitPipelines(vk::RenderPass outRp)
{
	MakeCopyHdrPipeline(outRp);
	MakeRtPipeline();
	m_postprocCollection.RegisterTechniques();
}

void Renderer_::MakeCopyHdrPipeline(vk::RenderPass outRp)
{
	GpuAsset<Shader>& gpuShader = GpuAssetManager->CompileShader("engine-data/spv/cpyhdr.shader");
	gpuShader.onCompile = [=]() {
		MakeCopyHdrPipeline(outRp);
	};

	std::vector shaderStages = gpuShader.shaderStages;

	// fixed-function stage
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

	vk::PipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer
		.setDepthClampEnable(VK_FALSE) //
		.setRasterizerDiscardEnable(VK_FALSE)
		.setPolygonMode(vk::PolygonMode::eFill)
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
		.setBlendEnable(VK_TRUE)
		.setSrcColorBlendFactor(vk::BlendFactor::eOne)
		.setDstColorBlendFactor(vk::BlendFactor::eOne)
		.setColorBlendOp(vk::BlendOp::eAdd)
		.setSrcAlphaBlendFactor(vk::BlendFactor::eOne)
		.setDstAlphaBlendFactor(vk::BlendFactor::eOne)
		.setAlphaBlendOp(vk::BlendOp::eAdd);

	vk::PipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending
		.setLogicOpEnable(VK_FALSE) //
		.setLogicOp(vk::LogicOp::eCopy)
		.setAttachmentCount(1u)
		.setPAttachments(&colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });


	// dynamic states
	vk::DynamicState dynamicStates[] = { vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo
		.setDynamicStateCount(2u) //
		.setPDynamicStates(dynamicStates);

	std::array layouts = { Layouts->singleSamplerDescLayout.setLayout.get() };

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setSetLayoutCount(static_cast<uint32>(layouts.size())) //
		.setPSetLayouts(layouts.data())
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
		// WIP: decouple
		.setRenderPass(outRp)
		.setSubpass(0u)
		.setBasePipelineHandle({})
		.setBasePipelineIndex(-1);

	m_pipeline = Device->createGraphicsPipelineUnique(nullptr, pipelineInfo);
}

void Renderer_::MakeRtPipeline()
{
	// all rt shaders here
	GpuAsset<Shader>& shader = GpuAssetManager->CompileShader("engine-data/spv/raytrace/test.shader");
	shader.onCompile = [&]() {
		MakeRtPipeline();
	};

	// Indices within this vector will be used as unique identifiers for the shaders in the Shader Binding Table.
	std::vector<vk::PipelineShaderStageCreateInfo> stages;

	// Raygen
	vk::RayTracingShaderGroupCreateInfoKHR rg{};
	rg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eRaygenKHR, raygenSM, "main" });
	rg.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	// m_rtShaderGroups.push_back(rg);

	// Miss
	vk::RayTracingShaderGroupCreateInfoKHR mg{};
	mg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eMissKHR, missSM, "main" });
	rg.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	// m_rtShaderGroups.push_back(mg);

	vk::RayTracingShaderGroupCreateInfoKHR hg{};
	hg.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eClosestHitKHR, chitSM, "main" });
	hg.setClosestHitShader(static_cast<uint32>(stages.size() - 1));

	// m_rtShaderGroups.push_back(hg);

	// Push constant: we want to be able to update constants used by the shaders
	vk::PushConstantRange pushConstant{};
	pushConstant
		.setStageFlags(vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR
					   | vk::ShaderStageFlagBits::eMissKHR) //
		.setOffset(0u)
		.setSize(sizeof(pushConstant));

	std::array layouts = {....};

	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setPushConstantRangeCount(1u) //
		.setPPushConstantRanges(&pushConstant)
		.setSetLayoutCount(static_cast<uint32>(layouts.size()))
		.setPSetLayouts(layouts.data());

	m_pipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);

	// Assemble the shader stages and recursion depth info into the ray tracing pipeline
	vk::RayTracingPipelineCreateInfoKHR rayPipelineInfo{};
	rayPipelineInfo
		.setStageCount(static_cast<uint32>(stages.size())) // Stages are shaders
		.setPStages(stages.data());

	rayPipelineInfo
		.setGroupCount(static_cast<uint32>(m_rtShaderGroups.size())) // 1-raygen, n-miss, n-(hit[+anyhit+intersect])
		.setPGroups(m_rtShaderGroups.data())

		// Note that it is preferable to keep the recursion level as low as possible, replacing it by a loop formulation
		// instead.

		.setMaxRecursionDepth(1) // Ray depth
		.setLayout(m_rtPipelineLayout.get());
	m_rtPipeline = Device->createRayTracingPipelineKHRUnique({}, rayPipelineInfo);
}

void Renderer_::CreateRtShaderBindingTable()
{
	auto groupCount = static_cast<uint32>(m_rtShaderGroups.size());     // 3 shaders: raygen, miss, chit
	uint32 groupHandleSize = Device->pd.rtProps.shaderGroupHandleSize;  // Size of a program identifier
	uint32 baseAlignment = Device->pd.rtProps.shaderGroupBaseAlignment; // Size of shader alignment

	// Fetch all the shader handles used in the pipeline, so that they can be written in the SBT
	uint32 sbtSize = groupCount * baseAlignment;

	std::vector<byte> shaderHandleStorage(sbtSize);
	Device->getRayTracingShaderGroupHandlesKHR(m_rtPipeline.get(), 0, groupCount, sbtSize, shaderHandleStorage.data());
	// Write the handles in the SBT
	RBuffer rtSBTBuffer = RBuffer{ sbtSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };
	// m_debug.setObjectName(m_rtSBTBuffer.buffer, std::string("SBT").c_str());

	rtSBTBuffer.UploadData(shaderHandleStorage);
}

void Renderer_::RecordGeometryPasses(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	auto& extent = m_gbuffer[sceneDesc.frameIndex].framebuffer.extent;

	vk::Rect2D scissor{};
	scissor
		.setOffset({ 0, 0 }) //
		.setExtent(vk::Extent2D{ extent.width, extent.height });

	vk::Viewport viewport{};
	viewport
		.setX(0) //
		.setY(0)
		.setWidth(static_cast<float>(extent.width))
		.setHeight(static_cast<float>(extent.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);


	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(Layouts->gbufferPass.get()) //
		.setFramebuffer(m_gbuffer[sceneDesc.frameIndex].framebuffer);
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	std::array<vk::ClearValue, 6> clearValues = {};
	clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[1].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[2].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[3].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[4].setDepthStencil({ 1.0f, 0 });
	renderPassInfo
		.setClearValueCount(static_cast<uint32>(clearValues.size())) //
		.setPClearValues(clearValues.data());


	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
	{
		auto buffer = m_secondaryBuffersPool.Get(sceneDesc.frameIndex);

		GbufferPass::RecordCmd(&buffer, viewport, scissor, sceneDesc);

		cmdBuffer->executeCommands({ buffer });
	}
	cmdBuffer->endRenderPass();

	auto shadowmapRenderpass = [&](auto light) {
		if (light) {

			auto& extent = light->shadowmap[sceneDesc.frameIndex].framebuffer.extent;

			vk::Rect2D scissor{};

			scissor
				.setOffset({ 0, 0 }) //
				.setExtent(extent);

			auto vpSize = extent;

			vk::Viewport viewport{};
			viewport
				.setX(0) //
				.setY(0)
				.setWidth(static_cast<float>(vpSize.width))
				.setHeight(static_cast<float>(vpSize.height))
				.setMinDepth(0.f)
				.setMaxDepth(1.f);

			vk::RenderPassBeginInfo renderPassInfo{};
			renderPassInfo
				.setRenderPass(Layouts->depthRenderPass.get()) //
				.setFramebuffer(light->shadowmap[sceneDesc.frameIndex].framebuffer);
			renderPassInfo.renderArea
				.setOffset({ 0, 0 }) //
				.setExtent(extent);

			vk::ClearValue clearValues = {};
			clearValues.setDepthStencil({ 1.0f, 0 });
			renderPassInfo
				.setClearValueCount(1u) //
				.setPClearValues(&clearValues);

			cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
			{
				auto buffer = m_secondaryBuffersPool.Get(sceneDesc.frameIndex);

				DepthmapPass::RecordCmd(&buffer, viewport, scissor, light->ubo.viewProj, sceneDesc);

				cmdBuffer->executeCommands({ buffer });
			}
			cmdBuffer->endRenderPass();
		}
	};

	for (auto sl : sceneDesc->spotlights.elements) {
		shadowmapRenderpass(sl);
	}

	for (auto dl : sceneDesc->directionalLights.elements) {
		shadowmapRenderpass(dl);
	}
}

void Renderer_::RecordRayTracingPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	// NEXT: temp
	// write all geometry, indices to this desc set... how to combine them tho?
	vk::DescriptorBufferInfo bufferInfo{};

	bufferInfo
		.setBuffer(buffer[i]) //
		.setOffset(0u)
		.setRange(GetBufferSize());
	vk::WriteDescriptorSet descriptorWrite{};

	descriptorWrite
		.setDstSet(descSet[i]) //
		.setDstBinding(0u)
		.setDstArrayElement(0u)
		.setDescriptorType(vk::DescriptorType::eStorageBuffer)
		.setDescriptorCount(1u)
		.setPBufferInfo(&bufferInfo)
		.setPImageInfo(nullptr)
		.setPTexelBufferView(nullptr);

	vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);


	// Initializing push constant values
	// WIP: what about secondary buffers?
	cmdBuffer->executeCommands({ buffer });


	cmdBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipeline.get());
	cmdBuffer->bindDescriptorSets(
		vk::PipelineBindPoint::eRayTracingKHR, m_rtPipelineLayout, 0, { m_rtDescSet, m_descSet }, {});
	// cmdBuf.pushConstants<RtPushConstant>(m_rtPipelineLayout,
	//	vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR
	//		| vk::ShaderStageFlagBits::eMissKHR,
	//	0, m_rtPushConstants);

	vk::DeviceSize progSize = Device->pd.rtProps.shaderGroupBaseAlignment; // Size of a program identifier
	vk::DeviceSize rayGenOffset = 0u * progSize;                           // Start at the beginning of m_sbtBuffer
	vk::DeviceSize missOffset = 1u * progSize;                             // Jump over raygen
	vk::DeviceSize missStride = progSize;
	vk::DeviceSize hitGroupOffset = 2u * progSize; // Jump over the previous shaders
	vk::DeviceSize hitGroupStride = progSize;

	// We can finally call traceRaysKHR that will add the ray tracing launch in the command buffer. Note that the SBT
	// buffer is mentioned several times. This is due to the possibility of separating the SBT into several buffers, one
	// for each type: ray generation, miss shaders, hit groups, and callable shaders (outside the scope of this
	// tutorial). The last three parameters are equivalent to the grid size of a compute launch, and represent the total
	// number of threads. Since we want to trace one ray per pixel, the grid size has the width and height of the output
	// image, and a depth of 1.

	vk::DeviceSize sbtSize = progSize * (vk::DeviceSize)m_rtShaderGroups.size();

	const vk::StridedBufferRegionKHR raygenShaderBindingTable = { m_rtSBTBuffer, rayGenOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR missShaderBindingTable = { m_rtSBTBuffer, missOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR hitShaderBindingTable = { m_rtSBTBuffer, hitGroupOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR callableShaderBindingTable;


	auto& extent = m_gbuffer[sceneDesc.frameIndex].framebuffer.extent;

	cmdBuffer->traceRaysKHR(&raygenShaderBindingTable, &missShaderBindingTable, &hitShaderBindingTable,
		&callableShaderBindingTable, extent.width, extent.height, 1);
}

void Renderer_::RecordPostProcessPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	auto& extent = m_gbuffer[sceneDesc.frameIndex].framebuffer.extent;

	vk::Rect2D scissor{};
	scissor
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	vk::Viewport viewport{};
	viewport
		.setX(0) //
		.setY(0)
		.setWidth(static_cast<float>(extent.width))
		.setHeight(static_cast<float>(extent.height))
		.setMinDepth(0.f)
		.setMaxDepth(1.f);


	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(m_ptRenderpass.get()) //
		.setFramebuffer(m_framebuffer[sceneDesc.frameIndex].get());
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	vk::ClearValue clearValue{};
	clearValue.setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });

	vk::ClearValue clearValue2{};
	clearValue2.setColor(std::array{ 0.2f, 0.2f, 0.0f, 1.0f });

	std::array cv{ clearValue, clearValue2 };

	renderPassInfo
		.setClearValueCount(static_cast<uint32>(cv.size())) //
		.setPClearValues(cv.data());

	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	{
		cmdBuffer->setViewport(0, { viewport });
		cmdBuffer->setScissor(0, { scissor });

		m_postprocCollection.Draw(*cmdBuffer, sceneDesc, m_gbuffer[sceneDesc.frameIndex].descSet); // WIP:

		UnlitPass::RecordCmd(cmdBuffer, sceneDesc);
	}
	cmdBuffer->endRenderPass();
}

void Renderer_::RecordOutPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
	vk::Framebuffer outFb, vk::Extent2D outExtent)
{
	PROFILE_SCOPE(Renderer);

	vk::RenderPassBeginInfo renderPassInfo{};
	renderPassInfo
		.setRenderPass(outRp) //
		.setFramebuffer(outFb);
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(outExtent);

	vk::ClearValue clearValue{};
	clearValue.setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	renderPassInfo
		.setClearValueCount(1u) //
		.setPClearValues(&clearValue);

	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eInline);
	{
		auto& scissor = m_viewportRect;
		const float x = static_cast<float>(scissor.offset.x);
		const float y = static_cast<float>(scissor.offset.y);
		const float width = static_cast<float>(scissor.extent.width);
		const float height = static_cast<float>(scissor.extent.height);

		vk::Viewport viewport{};
		viewport
			.setX(x) //
			.setY(y)
			.setWidth(width)
			.setHeight(height)
			.setMinDepth(0.f)
			.setMaxDepth(1.f);

		cmdBuffer->setViewport(0, { viewport });
		cmdBuffer->setScissor(0, { scissor });

		// Copy hdr texture
		{
			cmdBuffer->bindPipeline(vk::PipelineBindPoint::eGraphics, m_pipeline.get());
			cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eGraphics, m_pipelineLayout.get(), 0u, 1u,
				&Renderer->m_ppDescSet[sceneDesc.frameIndex], 0u, nullptr);
			// big triangle
			cmdBuffer->draw(3u, 1u, 0u, 0u);
		}

		ImguiImpl::RenderVulkan(cmdBuffer);
	}
	cmdBuffer->endRenderPass();
}

void Renderer_::OnViewportResize()
{
	vk::Extent2D viewportSize{ g_ViewportCoordinates.size.x, g_ViewportCoordinates.size.y };

	m_viewportRect.extent = viewportSize;
	m_viewportRect.offset = vk::Offset2D(g_ViewportCoordinates.position.x, g_ViewportCoordinates.position.y);

	vk::Extent2D fbSize = SuggestFramebufferSize(viewportSize);

	if (fbSize != m_viewportFramebufferSize) {
		m_viewportFramebufferSize = fbSize;

		for (uint32 i = 0; i < c_framesInFlight; ++i) {

			m_gbuffer[i] = GBuffer{ fbSize.width, fbSize.height };

			m_attachment[i] = RImageAttachment{ fbSize.width, fbSize.height, vk::Format::eR32G32B32A32Sfloat,
				vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled
					| vk::ImageUsageFlagBits::eInputAttachment,
				vk::MemoryPropertyFlagBits::eDeviceLocal, "rgba32" };

			m_attachment2[i] = RImageAttachment{ fbSize.width, fbSize.height, vk::Format::eR32G32B32A32Sfloat,
				vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eSampled,
				vk::MemoryPropertyFlagBits::eDeviceLocal, "rgba32" };

			// descSets
			auto quadSampler = GpuAssetManager->GetDefaultSampler();

			vk::DescriptorImageInfo imageInfo{};
			imageInfo
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				.setImageView(m_attachment2[i]())
				.setSampler(quadSampler);

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite
				.setDstSet(m_ppDescSet[i]) //
				.setDstBinding(0u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setDescriptorCount(1u)
				.setPBufferInfo(nullptr)
				.setPImageInfo(&imageInfo)
				.setPTexelBufferView(nullptr);

			Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);


			ptDebugObj->descSet[i] = ptDebugObj->descLayout.GetDescriptorSet();


			vk::DescriptorImageInfo imageInfo2{};
			imageInfo2
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				.setImageView(Renderer->m_attachment[i]());
			//	.setSampler(VK_NULL_HANDLE);

			vk::WriteDescriptorSet descriptorWrite2{};
			descriptorWrite2
				.setDstSet(ptDebugObj->descSet[i]) //
				.setDstBinding(0)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eInputAttachment)
				.setDescriptorCount(1u)
				.setPBufferInfo(nullptr)
				.setPImageInfo(&imageInfo2)
				.setPTexelBufferView(nullptr);

			Device->updateDescriptorSets(1u, &descriptorWrite2, 0u, nullptr);


			std::array<vk::ImageView, 3> attch{
				m_attachment[i](),
				m_attachment2[i](),
				m_gbuffer[i].framebuffer[GDepth](),
			};

			// framebuffer
			vk::FramebufferCreateInfo createInfo{};
			createInfo
				.setRenderPass(m_ptRenderpass.get()) //
				.setAttachmentCount(static_cast<uint32>(attch.size()))
				.setPAttachments(attch.data())
				.setWidth(fbSize.width)
				.setHeight(fbSize.height)
				.setLayers(1u);

			m_framebuffer[i] = Device->createFramebufferUnique(createInfo);
		}
	}
} // namespace vl

void Renderer_::PrepareForFrame()
{
	if (*m_didViewportResize) {
		OnViewportResize();
	}
}

void Renderer_::DrawFrame(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc, vk::RenderPass outRp,
	vk::Framebuffer outFb, vk::Extent2D outExtent)
{
	PROFILE_SCOPE(Renderer);

	m_secondaryBuffersPool.Top();

	// passes
	RecordGeometryPasses(cmdBuffer, sceneDesc);
	RecordPostProcessPass(cmdBuffer, sceneDesc);

	for (auto& att : m_gbuffer[sceneDesc.frameIndex].framebuffer.attachments) {
		if (att.isDepth) {
			continue;
		}
		att.TransitionForWrite(cmdBuffer);
	}

	for (auto sl : sceneDesc->spotlights.elements) {
		if (sl) {
			sl->shadowmap[sceneDesc.frameIndex].framebuffer[0].TransitionForWrite(cmdBuffer);
		}
	}

	for (auto dl : sceneDesc->directionalLights.elements) {
		if (dl) {
			dl->shadowmap[sceneDesc.frameIndex].framebuffer[0].TransitionForWrite(cmdBuffer);
		}
	}

	RecordOutPass(cmdBuffer, sceneDesc, outRp, outFb, outExtent);

	m_attachment[sceneDesc.frameIndex].TransitionForWrite(cmdBuffer);
	m_attachment2[sceneDesc.frameIndex].TransitionForWrite(cmdBuffer);
}
} // namespace vl
