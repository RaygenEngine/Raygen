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
#include "rendering/core/PipeUtl.h"


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
		vl::Renderer->m_rtDepth = std::max(0, depth);
		LOG_WARN("Rt depth set to: {}", vl::Renderer->m_rtDepth);
	},
	"Set rt depth" };

ConsoleFunction<int32> console_setRtSamples{ "rt.samples",
	[](int32 smpls) {
		vl::Renderer->m_rtSamples = std::max(0, smpls);
		LOG_WARN("Rt samples set to: {}", vl::Renderer->m_rtSamples);
	},
	"Set rt samples" };

ConsoleFunction<> console_resetRtFrame{ "rt.reset", []() { vl::Renderer->m_rtFrame = 0; }, "Reset rt frame" };

} // namespace

namespace vl {

void Renderer_::InitPipelines()
{
	m_postprocCollection.RegisterTechniques();
	MakeRtPipeline();
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
		vk::DescriptorImageInfo imageInfo{ {}, m_ptPass[i].framebuffer[1].view(), vk::ImageLayout::eGeneral };
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
		vk::DescriptorImageInfo imageInfo{ {}, m_ptPass[i].framebuffer[0].view(),
			vk::ImageLayout::eShaderReadOnlyOptimal };
		imageInfo.setSampler(GpuAssetManager->GetDefaultSampler());

		vk::WriteDescriptorSet descriptorWrite{};

		descriptorWrite
			.setDstSet(m_wipDescSet[i]) //
			.setDstBinding(0u)
			.setDstArrayElement(0u)
			.setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
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

	DEBUG_NAME_AUTO(m_rtDescSet[sceneDesc.frameIndex]);
	DEBUG_NAME_AUTO(sceneDesc.scene->sceneAsDescSet);
	DEBUG_NAME_AUTO(sceneDesc.viewer->descSet[sceneDesc.frameIndex]);
	DEBUG_NAME_AUTO(sceneDesc.scene->tlas.sceneDesc.descSet);
	DEBUG_NAME_AUTO(m_gbuffer[sceneDesc.frameIndex].descSet);
	DEBUG_NAME_AUTO(m_wipDescSet[sceneDesc.frameIndex]);

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
		.setRenderPass(m_ptPass[sceneDesc.frameIndex].GetRenderPass()) //
		.setFramebuffer(m_ptPass[sceneDesc.frameIndex].framebuffer.handle());
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

void Renderer_::ResizeBuffers(uint32 width, uint32 height)
{
	vk::Extent2D fbSize = SuggestFramebufferSize(vk::Extent2D{ width, height });

	if (fbSize == m_viewportFramebufferSize) {
		return;
	}
	m_viewportFramebufferSize = fbSize;


	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_gbuffer[i] = GBuffer{ fbSize.width, fbSize.height };

		auto& depthAtt = m_gbuffer[i].framebuffer[GColorAttachment::GDepth];

		m_ptPass[i] = Layouts->ptPassLayout.CreatePassInstance(fbSize.width, fbSize.height, { &depthAtt });
	}
	SetRtImage();
}

InFlightResources<vk::ImageView> Renderer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_ptPass[i].framebuffer[1].view();
	}
	return views;
}

void Renderer_::DrawFrame(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc, OutputPassBase& outputPass)
{
	PROFILE_SCOPE(Renderer);

	m_secondaryBuffersPool.Top();

	// passes
	RecordGeometryPasses(&cmdBuffer, sceneDesc);
	RecordPostProcessPass(&cmdBuffer, sceneDesc);


	static bool raytrace = true;

	if (Input.IsJustPressed(Key::V)) {
		raytrace = !raytrace;
	}

	if (raytrace) {
		m_ptPass[sceneDesc.frameIndex].framebuffer[1].TransitionToLayout(cmdBuffer,
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
			vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

		RecordRayTracingPass(&cmdBuffer, sceneDesc);

		m_ptPass[sceneDesc.frameIndex].framebuffer[1].TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
			vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
			vk::PipelineStageFlagBits::eFragmentShader);
	}


	for (auto& att : m_gbuffer[sceneDesc.frameIndex].framebuffer.ownedAttachments) {
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

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);

	m_ptPass[sceneDesc.frameIndex].TransitionFramebufferForWrite(cmdBuffer);
}
} // namespace vl
