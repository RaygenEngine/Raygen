#include "Renderer.h"

#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/output/OutputPassBase.h"
#include "rendering/pipes/AmbientPipe.h"
#include "rendering/pipes/BillboardPipe.h"
#include "rendering/pipes/DirlightPipe.h"
#include "rendering/pipes/geometry/GBufferPipe.h"
#include "rendering/pipes/geometry/UnlitPipe.h"
#include "rendering/pipes/IrragridPipe.h"
#include "rendering/pipes/PointlightPipe.h"
#include "rendering/pipes/ReflprobePipe.h"
#include "rendering/pipes/SpotlightPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/techniques/CalculateDynamicShadowmaps.h"
#include "rendering/techniques/DrawSelectedEntityDebugVolume.h"
#include "engine/Events.h"


namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

Renderer_::Renderer_()
{
	Event::OnViewerUpdated.Bind(this, [&]() { m_raytraceArealights.frame = 0; });

	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_globalDesc[i] = DescriptorLayouts->global.AllocDescriptorSet();
	}

	// m_postprocCollection.RegisterTechniques();

	m_ptLightBlend.MakeLayout();
	m_ptLightBlend.MakePipeline();
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
		m_mainPassInst[i] = PassLayouts->main.CreatePassInstance(fbSize.width, fbSize.height);
		m_secondaryPassInst[i] = PassLayouts->secondary.CreatePassInstance(fbSize.width, fbSize.height);
		m_ptPass[i] = PassLayouts->postproc.CreatePassInstance(fbSize.width, fbSize.height);
	}

	m_raytraceArealights.Resize(fbSize);
	m_raytraceMirrorReflections.Resize(fbSize);

	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_unlitPassInst[i] = PassLayouts->unlit.CreatePassInstance(fbSize.width, fbSize.height,
			{ &m_mainPassInst[i].framebuffer["G_Depth"], &m_ptPass[i].framebuffer["PostProcColor"] });
	}

	for (uint32 i = 0; i < c_framesInFlight; ++i) {

		std::vector<rvk::ImageSampler> imageSamplers;

		for (auto& att : m_mainPassInst[i].framebuffer.ownedAttachments) {
			imageSamplers.emplace_back(att.view());
		}

		auto [brdfLutImg, brdfLutSampler] = GpuAssetManager->GetBrdfLutImageSampler();
		imageSamplers.emplace_back(m_secondaryPassInst[i].framebuffer["Ambient"].view());
		imageSamplers.emplace_back(
			brdfLutImg.Lock().image.view(), brdfLutSampler); // std_BrdfLut <- rewritten below with the correct sampler
		imageSamplers.emplace_back(
			m_raytraceArealights.svgfRenderPassInstance.framebuffer["SVGF Result"].view()); // arealightShadowing
		imageSamplers.emplace_back(m_raytraceArealights.progressive.view());                // reserved1
		imageSamplers.emplace_back(m_raytraceMirrorReflections.result[i].view());           // mirror
		imageSamplers.emplace_back(m_ptPass[i].framebuffer["PostProcColor"].view());        // sceneColorSampler

		rvk::writeDescriptorImages(m_globalDesc[i], 0u, std::move(imageSamplers));
	}

	ClearDebugAttachments();
	RegisterDebugAttachment(m_mainPassInst.at(0).framebuffer);
	RegisterDebugAttachment(m_secondaryPassInst.at(0).framebuffer);
	RegisterDebugAttachment(m_ptPass.at(0).framebuffer);
	RegisterDebugAttachment(m_raytraceMirrorReflections.result.at(0));
	RegisterDebugAttachment(m_raytraceArealights.svgfRenderPassInstance.framebuffer);
}

InFlightResources<vk::ImageView> Renderer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_ptPass[i].framebuffer["PostProcColor"].view();
	}
	return views;
}

void Renderer_::UpdateGlobalDescSet(SceneRenderDesc& sceneDesc)
{
	if (m_viewerPtr != &sceneDesc.viewer) [[unlikely]] {
		Device->waitIdle();
		for (size_t i = 0; i < c_framesInFlight; ++i) {
			rvk::writeDescriptorBuffer(m_globalDesc[i], DescriptorLayouts_::GlobalIndex::Viewer,
				sceneDesc.viewer.buffer[i].handle(), sceneDesc.viewer.uboSize);
		}
		Device->waitIdle();
		m_viewerPtr = &sceneDesc.viewer;
	}

	sceneDesc.globalDesc = m_globalDesc[sceneDesc.frameIndex];
}

void Renderer_::DrawGeometryAndLights(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc)
{
	m_mainPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		GbufferPipe::RecordCmd(cmdBuffer, sceneDesc);

		auto inputDescSet = m_mainPassInst[sceneDesc.frameIndex].internalDescSet;

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		StaticPipes::Get<SpotlightPipe>().RecordCmd(cmdBuffer, sceneDesc, inputDescSet);

		StaticPipes::Get<PointlightPipe>().RecordCmd(cmdBuffer, sceneDesc, inputDescSet);

		StaticPipes::Get<DirlightPipe>().RecordCmd(cmdBuffer, sceneDesc, inputDescSet);

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		StaticPipes::Get<ReflprobePipe>().RecordCmd(cmdBuffer, sceneDesc, inputDescSet);

		StaticPipes::Get<IrragridPipe>().RecordCmd(cmdBuffer, sceneDesc, inputDescSet);
	});

	m_secondaryPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline,
		[&]() { StaticPipes::Get<AmbientPipe>().RecordCmd(cmdBuffer, sceneDesc); });
}

void Renderer_::RecordCmd(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass)
{
	PROFILE_SCOPE(Renderer);
	COMMAND_SCOPE_AUTO(cmdBuffer);

	UpdateGlobalDescSet(sceneDesc);

	// calculates: dynamic shadowmaps
	// requires: -
	CalculateDynamicShadowmaps::RecordCmd(cmdBuffer, sceneDesc);

	// calculates: gbuffer, direct lights, gi, ambient
	// requires: shadowmaps, gi maps
	DrawGeometryAndLights(cmdBuffer, sceneDesc);

	// calculates: total of arealights
	// requires: -
	m_raytraceArealights.RecordCmd(cmdBuffer, sceneDesc);

	// calculates: specular reflections (mirror)
	// requires: gbuffer, shadowmaps, gi maps
	m_raytraceMirrorReflections.RecordCmd(cmdBuffer, sceneDesc);

	// TODO: post process
	m_ptPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&] {
		m_ptLightBlend.RecordCmd(cmdBuffer, sceneDesc); // fixed

		// TODO: from post proc
		// m_postprocCollection.RecordCmd(*cmdBuffer, sceneDesc);
	});

	static ConsoleVariable<bool> cons_drawUnlit{ "r.renderer.drawUnlit", true }; // CHECK: more specific

	if (cons_drawUnlit) [[likely]] {
		m_unlitPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&] {
			UnlitPipe::RecordCmd(cmdBuffer, sceneDesc);
			DrawSelectedEntityDebugVolume::RecordCmd(cmdBuffer, sceneDesc);
			StaticPipes::Get<BillboardPipe>().RecordCmd(cmdBuffer, sceneDesc);
		});
	}


	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
