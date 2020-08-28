#include "pch.h"
#include "Renderer.h"

#include "rendering/Renderer.h"

#include "engine/console/ConsoleVariable.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/passes/DepthmapPass.h"
#include "rendering/passes/GBufferPass.h"
#include "rendering/passes/UnlitPass.h"
#include "rendering/ppt/techniques/PtDebug.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneDirectionalLight.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/structures/Depthmap.h"
#include "rendering/wrappers/Swapchain.h"

#include <editor/imgui/ImguiImpl.h>

namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}

struct PushConstant {
	int32 frame;
	int32 depth;
	int32 samples;
};
static_assert(sizeof(PushConstant) <= 128);

ConsoleFunction<int32> console_setRtDepth{ "rt.depth",
	[](int32 depth) {
		if (depth > 0) {
			vl::Renderer->m_rtDepth = depth;
		}
		else {
			vl::Renderer->m_rtDepth = 0;
		}
		LOG_WARN("Rt depth set to: {}", vl::Renderer->m_rtDepth);
	},
	"Set rt depth" };

ConsoleFunction<int32> console_setRtSamples{ "rt.samples",
	[](int32 smpls) {
		if (smpls > 0) {
			vl::Renderer->m_rtSamples = smpls;
		}
		else {
			vl::Renderer->m_rtSamples = 0;
		}
		LOG_WARN("Rt samples set to: {}", vl::Renderer->m_rtSamples);
	},
	"Set rt samples" };

ConsoleFunction<> console_resetRtFrame{ "rt.reset", []() { vl::Renderer->m_rtFrame = 0; }, "Reset rt frame" };

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
		.setColorAttachments(colorAttachmentRef)
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
		.setInputAttachments(colorAttachmentRef15)
		.setColorAttachments(colorAttachmentRef2)
		.setPDepthStencilAttachment(&depthAttachmentRef);

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
	std::array dependencies{ lightDep, debugDep };
	std::array attachments{ colorAttachmentDesc, colorAttachmentDesc2, depthAttachmentDesc };

	vk::RenderPassCreateInfo renderPassInfo{};
	renderPassInfo
		.setAttachments(attachments) //
		.setSubpasses(subpasses)
		.setDependencies(dependencies);

	m_ptRenderpass = Device->createRenderPassUnique(renderPassInfo);

	// descsets
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_ppDescSet[i] = Layouts->singleSamplerDescLayout.AllocDescriptorSet();
	}
}

void Renderer_::InitPipelines(vk::RenderPass outRp)
{
	MakeCopyHdrPipeline(outRp);

	m_postprocCollection.RegisterTechniques();
	MakeRtPipeline();
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
		.setViewports(viewport) //
		.setScissors(scissor);

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
		.setAttachments(colorBlendAttachment)
		.setBlendConstants({ 0.f, 0.f, 0.f, 0.f });


	// dynamic states
	std::array dynamicStates{ vk::DynamicState::eViewport, vk::DynamicState::eScissor };

	vk::PipelineDynamicStateCreateInfo dynamicStateInfo{};
	dynamicStateInfo.setDynamicStates(dynamicStates);

	// pipeline layout
	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.setSetLayouts(Layouts->singleSamplerDescLayout.handle());

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

	shader.onCompileRayTracing = [&]() {
		MakeRtPipeline();
	};
	m_rtShaderGroups.clear();

	// Indices within this vector will be used as unique identifiers for the shaders in the Shader Binding Table.
	std::vector<vk::PipelineShaderStageCreateInfo> stages;

	// Raygen
	vk::RayTracingShaderGroupCreateInfoKHR rg{};
	rg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eRaygenKHR, *shader.rayGen.Lock().module, "main" });
	rg.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(rg);

	// Miss
	vk::RayTracingShaderGroupCreateInfoKHR mg{};
	mg.setType(vk::RayTracingShaderGroupTypeKHR::eGeneral) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eMissKHR, *shader.miss.Lock().module, "main" });
	mg.setGeneralShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(mg);

	vk::RayTracingShaderGroupCreateInfoKHR hg{};
	hg.setType(vk::RayTracingShaderGroupTypeKHR::eTrianglesHitGroup) //
		.setGeneralShader(VK_SHADER_UNUSED_KHR)
		.setClosestHitShader(VK_SHADER_UNUSED_KHR)
		.setAnyHitShader(VK_SHADER_UNUSED_KHR)
		.setIntersectionShader(VK_SHADER_UNUSED_KHR);
	stages.push_back({ {}, vk::ShaderStageFlagBits::eClosestHitKHR, *shader.closestHit.Lock().module, "main" });
	hg.setClosestHitShader(static_cast<uint32>(stages.size() - 1));

	m_rtShaderGroups.push_back(hg);


	std::array layouts{
		Layouts->singleStorageImage.handle(),
		Layouts->accelLayout.handle(),
		Layouts->singleUboDescLayout.handle(),
		Layouts->rtSceneDescLayout.handle(),
		Layouts->gbufferDescLayout.handle(),
		Layouts->singleSamplerDescLayout.handle(),
	};

	// pipeline layout
	vk::PushConstantRange pushConstantRange{};
	pushConstantRange
		.setStageFlags(vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR) //
		.setSize(sizeof(PushConstant))
		.setOffset(0u);


	vk::PipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo
		.setPushConstantRanges(pushConstantRange) //
		.setSetLayouts(layouts);

	m_rtPipelineLayout = Device->createPipelineLayoutUnique(pipelineLayoutInfo);

	// Assemble the shader stages and recursion depth info into the ray tracing pipeline
	vk::RayTracingPipelineCreateInfoKHR rayPipelineInfo{};
	rayPipelineInfo
		// Stages are shaders
		.setStages(stages);

	rayPipelineInfo
		// 1-raygen, n-miss, n-(hit[+anyhit+intersect])
		.setGroups(m_rtShaderGroups)
		// Note that it is preferable to keep the recursion level as low as possible, replacing it by a loop formulation
		// instead.

		.setMaxRecursionDepth(10) // Ray depth TODO:
		.setLayout(m_rtPipelineLayout.get());
	m_rtPipeline = Device->createRayTracingPipelineKHRUnique({}, rayPipelineInfo);


	CreateRtShaderBindingTable();
	//

	// NEXT: temp
	// write all geometry, indices to this desc set... how to combine them tho?
}

void Renderer_::SetRtImage()
{
	m_rtDescSet = { Layouts->singleStorageImage.AllocDescriptorSet(), Layouts->singleStorageImage.AllocDescriptorSet(),
		Layouts->singleStorageImage.AllocDescriptorSet() };
	for (size_t i = 0; i < c_framesInFlight; i++) {
		vk::DescriptorImageInfo imageInfo{ {}, m_attachment2[i].view(), vk::ImageLayout::eGeneral };
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(m_rtDescSet[i]) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eStorageImage)
			.setImageInfo(imageInfo);

		vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}


	m_wipDescSet = { Layouts->singleSamplerDescLayout.AllocDescriptorSet(),
		Layouts->singleSamplerDescLayout.AllocDescriptorSet(), Layouts->singleSamplerDescLayout.AllocDescriptorSet() };
	for (size_t i = 0; i < c_framesInFlight; i++) {
		vk::DescriptorImageInfo imageInfo{ GpuAssetManager->GetDefaultSampler(), m_attachment[i].view(),
			vk::ImageLayout::eShaderReadOnlyOptimal };
		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(m_wipDescSet[i]) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eSampledImage)
			.setImageInfo(imageInfo);

		vl::Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);
	}
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
	m_rtSBTBuffer = RBuffer{ sbtSize, vk::BufferUsageFlagBits::eTransferSrc,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	DEBUG_NAME(m_rtSBTBuffer.handle(), "Shader Binding Table");


	// TODO: Tidy
	auto mem = m_rtSBTBuffer.memory();

	void* dptr = Device->mapMemory(mem, 0, sbtSize);

	auto* pData = reinterpret_cast<uint8_t*>(dptr);
	for (uint32_t g = 0; g < groupCount; g++) {
		memcpy(pData, shaderHandleStorage.data() + g * groupHandleSize, groupHandleSize);
		pData += baseAlignment;
	}
	Device->unmapMemory(mem);
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
		.setFramebuffer(m_gbuffer[sceneDesc.frameIndex].framebuffer.handle());
	renderPassInfo.renderArea
		.setOffset({ 0, 0 }) //
		.setExtent(extent);

	std::array<vk::ClearValue, 6> clearValues = {};
	clearValues[0].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[1].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[2].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[3].setColor(std::array{ 0.0f, 0.0f, 0.0f, 1.0f });
	clearValues[4].setDepthStencil({ 1.0f, 0 });
	renderPassInfo.setClearValues(clearValues);


	cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
	{
		auto buffers = m_secondaryBuffersPool.Get(sceneDesc.frameIndex);

		GbufferPass::RecordCmd(&buffers, viewport, scissor, sceneDesc);

		cmdBuffer->executeCommands({ buffers });
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
				.setFramebuffer(light->shadowmap[sceneDesc.frameIndex].framebuffer.handle());
			renderPassInfo.renderArea
				.setOffset({ 0, 0 }) //
				.setExtent(extent);

			vk::ClearValue clearValues{};
			clearValues.setDepthStencil({ 1.0f, 0 });
			renderPassInfo.setClearValues(clearValues);

			cmdBuffer->beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
			{
				auto buffers = m_secondaryBuffersPool.Get(sceneDesc.frameIndex);

				DepthmapPass::RecordCmd(&buffers, viewport, scissor, light->ubo.viewProj, sceneDesc);

				cmdBuffer->executeCommands({ buffers });
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


	// Initializing push constant values
	// WIP: what about secondary buffers?
	// cmdBuffer->executeCommands({ buffer });


	cmdBuffer->bindPipeline(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipeline.get());


	cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipelineLayout.get(), 0u, 1u,
		&m_rtDescSet[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipelineLayout.get(), 1u, 1u,
		&sceneDesc.scene->sceneAsDescSet, 0u, nullptr);

	cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipelineLayout.get(), 2u, 1u,
		&sceneDesc.viewer->descSet[sceneDesc.frameIndex], 0u, nullptr);

	cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipelineLayout.get(), 3u, 1u,
		&sceneDesc.scene->tlas.sceneDesc.descSet, 0u, nullptr);


	cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipelineLayout.get(), 4u, 1u,
		&m_gbuffer[sceneDesc.frameIndex].descSet, 0u, nullptr);

	cmdBuffer->bindDescriptorSets(vk::PipelineBindPoint::eRayTracingKHR, m_rtPipelineLayout.get(), 5u, 1u,
		&m_wipDescSet[sceneDesc.frameIndex], 0u, nullptr);

	PushConstant pc{ //
		m_rtFrame, m_rtDepth, m_rtSamples
	};

	++m_rtFrame;

	// Submit via push constant (rather than a UBO)
	cmdBuffer->pushConstants(m_rtPipelineLayout.get(),
		vk::ShaderStageFlagBits::eRaygenKHR | vk::ShaderStageFlagBits::eClosestHitKHR, 0u, sizeof(PushConstant), &pc);


	vk::DeviceSize progSize = Device->pd.rtProps.shaderGroupBaseAlignment; // Size of a program identifier

	// RayGen index
	vk::DeviceSize rayGenOffset = 0u * progSize; // Start at the beginning of m_sbtBuffer

	// Miss index
	vk::DeviceSize missOffset = 1u * progSize; // Jump over raygen
	vk::DeviceSize missStride = progSize;

	// Hit index
	vk::DeviceSize hitGroupOffset = 2u * progSize; // Jump over the previous shaders
	vk::DeviceSize hitGroupStride = progSize;


	// We can finally call traceRaysKHR that will add the ray tracing launch in the command buffer. Note that the
	// SBT buffer is mentioned several times. This is due to the possibility of separating the SBT into several
	// buffers, one for each type: ray generation, miss shaders, hit groups, and callable shaders (outside the scope
	// of this tutorial). The last three parameters are equivalent to the grid size of a compute launch, and
	// represent the total number of threads. Since we want to trace one ray per pixel, the grid size has the width
	// and height of the output image, and a depth of 1.

	vk::DeviceSize sbtSize = progSize * (vk::DeviceSize)m_rtShaderGroups.size();

	const vk::StridedBufferRegionKHR raygenShaderBindingTable
		= { m_rtSBTBuffer.handle(), rayGenOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR missShaderBindingTable = { m_rtSBTBuffer.handle(), missOffset, progSize, sbtSize };
	const vk::StridedBufferRegionKHR hitShaderBindingTable
		= { m_rtSBTBuffer.handle(), hitGroupOffset, progSize, sbtSize };
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
	renderPassInfo.setClearValues(cv);

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
	renderPassInfo.setClearValues(clearValue);

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
					| vk::ImageUsageFlagBits::eStorage | vk::ImageUsageFlagBits::eInputAttachment,
				vk::MemoryPropertyFlagBits::eDeviceLocal, "rgba32" };

			m_attachment2[i] = RImageAttachment{ fbSize.width, fbSize.height, vk::Format::eR32G32B32A32Sfloat,
				vk::ImageTiling::eOptimal, vk::ImageLayout::eUndefined,
				vk::ImageUsageFlagBits::eColorAttachment | vk::ImageUsageFlagBits::eStorage
					| vk::ImageUsageFlagBits::eSampled,
				vk::MemoryPropertyFlagBits::eDeviceLocal, "rgba32" };

			// descSets
			auto quadSampler = GpuAssetManager->GetDefaultSampler();

			vk::DescriptorImageInfo imageInfo{};
			imageInfo
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				.setImageView(m_attachment2[i].view())
				.setSampler(quadSampler);

			vk::WriteDescriptorSet descriptorWrite{};
			descriptorWrite
				.setDstSet(m_ppDescSet[i]) //
				.setDstBinding(0u)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
				.setImageInfo(imageInfo);

			Device->updateDescriptorSets(1u, &descriptorWrite, 0u, nullptr);


			ptDebugObj->descSet[i] = ptDebugObj->descLayout.AllocDescriptorSet();


			vk::DescriptorImageInfo imageInfo2{};
			imageInfo2
				.setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal) //
				.setImageView(Renderer->m_attachment[i].view());
			//	.setSampler(VK_NULL_HANDLE);

			vk::WriteDescriptorSet descriptorWrite2{};
			descriptorWrite2
				.setDstSet(ptDebugObj->descSet[i]) //
				.setDstBinding(0)
				.setDstArrayElement(0u)
				.setDescriptorType(vk::DescriptorType::eInputAttachment)
				.setImageInfo(imageInfo2);

			Device->updateDescriptorSets(1u, &descriptorWrite2, 0u, nullptr);


			std::array<vk::ImageView, 3> attch{
				m_attachment[i].view(),
				m_attachment2[i].view(),
				m_gbuffer[i].framebuffer[GDepth].view(),
			};

			DEBUG_NAME(m_attachment[i].handle(), "attachment 1: " + std::to_string(i));
			DEBUG_NAME(m_attachment2[i].handle(), "attachment 2: " + std::to_string(i));

			// framebuffer
			vk::FramebufferCreateInfo createInfo{};
			createInfo
				.setRenderPass(m_ptRenderpass.get()) //
				.setAttachments(attch)
				.setWidth(fbSize.width)
				.setHeight(fbSize.height)
				.setLayers(1u);

			m_framebuffer[i] = Device->createFramebufferUnique(createInfo);
		}
		SetRtImage();
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


	static bool raytrace = true;

	if (Input.IsJustPressed(Key::V)) {
		raytrace = !raytrace;
	}

	if (raytrace) {

		m_attachment[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eFragmentShader,
			vk::PipelineStageFlagBits::eRayTracingShaderKHR);

		m_attachment2[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eFragmentShader,
			vk::PipelineStageFlagBits::eRayTracingShaderKHR);


		RecordRayTracingPass(cmdBuffer, sceneDesc);

		m_attachment[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal,
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
			vk::PipelineStageFlagBits::eFragmentShader);

		m_attachment2[sceneDesc.frameIndex].TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
			vk::PipelineStageFlagBits::eFragmentShader);
	}


	for (auto& att : m_gbuffer[sceneDesc.frameIndex].framebuffer) {
		if (att.isDepth) {
			continue;
		}
		att.TransitionToLayout(
			cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
	}

	for (auto sl : sceneDesc->spotlights.elements) {
		if (sl) {
			sl->shadowmap[sceneDesc.frameIndex].framebuffer[0].TransitionToLayout(
				cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		}
	}

	for (auto dl : sceneDesc->directionalLights.elements) {
		if (dl) {
			dl->shadowmap[sceneDesc.frameIndex].framebuffer[0].TransitionToLayout(
				cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eDepthStencilAttachmentOptimal);
		}
	}

	RecordOutPass(cmdBuffer, sceneDesc, outRp, outFb, outExtent);


	m_attachment[sceneDesc.frameIndex].TransitionToLayout(
		cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
	m_attachment2[sceneDesc.frameIndex].TransitionToLayout(
		cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eColorAttachmentOptimal);
}
} // namespace vl
