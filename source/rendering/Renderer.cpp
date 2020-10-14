#include "pch.h"
#include "Renderer.h"

#include "engine/console/ConsoleVariable.h"
#include "engine/Engine.h"
#include "engine/Events.h"
#include "engine/Input.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuMesh.h"
#include "rendering/assets/GpuShader.h"
#include "rendering/assets/GpuShaderStage.h"
#include "rendering/core/PipeUtl.h"
#include "rendering/Device.h"
#include "rendering/Instance.h"
#include "rendering/Layouts.h"
#include "rendering/passes/DepthmapPass.h"
#include "rendering/passes/GBufferPass.h"
#include "rendering/passes/lightblend/DirlightBlend.h"
#include "rendering/passes/lightblend/PointlightBlend.h"
#include "rendering/passes/lightblend/SpotlightBlend.h"
#include "rendering/passes/UnlitPass.h"
#include "rendering/ppt/techniques/PtDebug.h"
#include "rendering/scene/Scene.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/structures/Depthmap.h"
#include "rendering/util/WriteDescriptorSets.h"
#include "rendering/wrappers/Swapchain.h"


namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

void Renderer_::InitPipelines()
{
	// m_postprocCollection.RegisterTechniques();

	lightblendPass.MakeLayout();
	lightblendPass.MakePipeline();

	m_raytracingPass.MakeRtPipeline();
}

void Renderer_::RecordGeometryPasses(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	m_gbufferInst[sceneDesc.frameIndex].RecordPass(*cmdBuffer, vk::SubpassContents::eInline, [&]() {
		//
		GbufferPass::RecordCmd(cmdBuffer, sceneDesc);
	});

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

void Renderer_::RecordRasterDirectPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	m_rasterDirectPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		SpotlightBlend::Draw(cmdBuffer, sceneDesc);
		PointlightBlend::Draw(cmdBuffer, sceneDesc);
		DirlightBlend::Draw(cmdBuffer, sceneDesc);
	});
}

void Renderer_::RecordPostProcessPass(vk::CommandBuffer* cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	m_ptPass[sceneDesc.frameIndex].RecordPass(*cmdBuffer, vk::SubpassContents::eInline, [&] {
		// Post proc pass
		lightblendPass.Draw(*cmdBuffer, sceneDesc); // WIP: from post proc

		// m_postprocCollection.Draw(*cmdBuffer, sceneDesc);
		UnlitPass::RecordCmd(cmdBuffer, sceneDesc);
	});
}

void Renderer_::ResizeBuffers(uint32 width, uint32 height)
{
	vk::Extent2D fbSize = SuggestFramebufferSize(vk::Extent2D{ width, height });

	if (fbSize == m_extent) {
		return;
	}
	m_extent = fbSize;


	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		// Generate Passes
		m_gbufferInst[i] = Layouts->gbufferPassLayout.CreatePassInstance(fbSize.width, fbSize.height);

		m_rasterDirectPass[i] = Layouts->rasterDirectPassLayout.CreatePassInstance(fbSize.width, fbSize.height);


		m_ptPass[i] = Layouts->ptPassLayout.CreatePassInstance(
			fbSize.width, fbSize.height, { &m_gbufferInst[i].framebuffer[GDepth] });
	}

	m_raytracingPass.Resize(fbSize);


	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_attachmentsDesc[i] = Layouts->renderAttachmentsLayout.AllocDescriptorSet();

		std::vector<vk::ImageView> views;

		for (auto& att : m_gbufferInst[i].framebuffer.ownedAttachments) {
			views.emplace_back(att.view());
		}

		views.emplace_back(m_rasterDirectPass[i].framebuffer[0].view());                        // rasterDirectSampler
		views.emplace_back(m_raytracingPass.m_svgfRenderPassInstance[i].framebuffer[0].view()); // rtIndirectSampler
		views.emplace_back(m_ptPass[i].framebuffer[0].view());                                  // sceneColorSampler

		rvk::writeDescriptorImages(m_attachmentsDesc[i], 0u, std::move(views));
	}

	// RT images
	for (size_t i = 0; i < c_framesInFlight; i++) {
		m_rtDescSet[i] = Layouts->tripleStorageImage.AllocDescriptorSet();

		rvk::writeDescriptorImages(m_rtDescSet[i], 0u,
			{ m_raytracingPass.svgfPass.swappingImages[0].view(), m_raytracingPass.m_progressiveResult.view(),
				m_raytracingPass.m_momentsBuffer.view() },
			vk::DescriptorType::eStorageImage, nullptr, vk::ImageLayout::eGeneral);
	}
}

InFlightResources<vk::ImageView> Renderer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_ptPass[i].framebuffer[0].view();
	}
	return views;
}

void Renderer_::DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc, OutputPassBase& outputPass)
{
	PROFILE_SCOPE(Renderer);

	m_secondaryBuffersPool.Top();


	sceneDesc.attDesc = m_attachmentsDesc[sceneDesc.frameIndex];

	// passes
	RecordGeometryPasses(&cmdBuffer, sceneDesc);
	RecordRasterDirectPass(cmdBuffer, sceneDesc);


	static bool raytrace = true;

	// if (Input.IsJustPressed(Key::V)) {
	//	raytrace = !raytrace;
	//}

	// if (raytrace) {
	//	m_raytracingPass.RecordPass(cmdBuffer, sceneDesc, this);
	//}


	RecordPostProcessPass(&cmdBuffer, sceneDesc);
	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
