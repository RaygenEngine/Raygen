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
#include "rendering/util/WriteDescriptorSets.h"


namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

void Renderer_::InitPipelines()
{
	m_postprocCollection.RegisterTechniques();
	m_raytracingPass.MakeRtPipeline();
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

void Renderer_::RecordPostProcessPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	m_ptPass[sceneDesc.frameIndex].RecordPass(*cmdBuffer, vk::SubpassContents::eInline, [&] {
		// Post proc pass
		m_postprocCollection.Draw(*cmdBuffer, sceneDesc, m_gbuffer[sceneDesc.frameIndex].descSet);
		UnlitPass::RecordCmd(cmdBuffer, sceneDesc);
	});
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

	// RT images
	for (size_t i = 0; i < c_framesInFlight; i++) {
		m_rtDescSet[i] = Layouts->singleStorageImage.AllocDescriptorSet();
		m_rasterLightDescSet[i] = Layouts->singleSamplerDescLayout.AllocDescriptorSet();


		rvk::writeDescriptorImages(m_rtDescSet[i], 0u, { m_ptPass[i].framebuffer[1].view() },
			vk::DescriptorType::eStorageImage, nullptr, vk::ImageLayout::eGeneral);

		rvk::writeDescriptorImages(m_rasterLightDescSet[i], 0u, { m_ptPass[i].framebuffer[0].view() });
	}
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

		m_raytracingPass.RecordPass(cmdBuffer, sceneDesc, this);

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
