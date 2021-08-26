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
#include "rendering/techniques/BakeProbes.h"
#include "rendering/techniques/CalculateShadowmaps.h"
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
		m_globalDesc[i] = Layouts->globalDescLayout.AllocDescriptorSet();
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
		m_mainPassInst[i] = Layouts->mainPassLayout.CreatePassInstance(fbSize.width, fbSize.height);
		m_secondaryPassInst[i] = Layouts->secondaryPassLayout.CreatePassInstance(fbSize.width, fbSize.height);
		m_ptPass[i] = Layouts->ptPassLayout.CreatePassInstance(fbSize.width, fbSize.height);
	}

	m_raytraceArealights.Resize(fbSize);
	m_raytraceMirrorReflections.Resize(fbSize);

	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_unlitPassInst[i] = Layouts->unlitPassLayout.CreatePassInstance(fbSize.width, fbSize.height,
			{ &m_mainPassInst[i].framebuffer[0], &m_ptPass[i].framebuffer[0] }); // TODO: indices and stuff
	}

	for (uint32 i = 0; i < c_framesInFlight; ++i) {

		std::vector<vk::ImageView> views;

		for (auto& att : m_mainPassInst[i].framebuffer.ownedAttachments) {
			views.emplace_back(att.view());
		}

		auto [brdfLutImg, brdfLutSampler] = GpuAssetManager->GetBrdfLutImageSampler();
		views.emplace_back(m_secondaryPassInst[i].framebuffer[0].view());
		views.emplace_back(brdfLutImg.Lock().image.view()); // std_BrdfLut <- rewritten below with the correct sampler
		views.emplace_back(m_raytraceArealights.svgfRenderPassInstance.framebuffer[0].view()); // reserved0
		views.emplace_back(m_raytraceArealights.progressive.view());                           // reserved1
		views.emplace_back(m_raytraceMirrorReflections.result[i].view());                      // mirror buffer
		views.emplace_back(m_ptPass[i].framebuffer[0].view());                                 // sceneColorSampler

		rvk::writeDescriptorImages(m_globalDesc[i], 0u, std::move(views));
		// TODO: should not rewrite, instead pass pairs with their sampler, if sampler is empty then default
		rvk::writeDescriptorImages(m_globalDesc[i], 11u, { brdfLutImg.Lock().image.view() }, brdfLutSampler);
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
		views[i] = m_ptPass[i].framebuffer[0].view(); // TODO: [0]
	}
	return views;
}

void Renderer_::UpdateGlobalDescSet(SceneRenderDesc& sceneDesc)
{
	if (m_viewerPtr != &sceneDesc.viewer) [[unlikely]] {
		for (size_t i = 0; i < c_framesInFlight; ++i) {
			rvk::writeDescriptorBuffer(m_globalDesc[i], 16u, sceneDesc.viewer.buffer[i].handle(),
				sceneDesc.viewer.uboSize); // TODO: binding index
		}
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

		StaticPipes::Get<SpotlightPipe>().Draw(cmdBuffer, sceneDesc, inputDescSet);

		StaticPipes::Get<PointlightPipe>().Draw(cmdBuffer, sceneDesc, inputDescSet);

		StaticPipes::Get<DirlightPipe>().Draw(cmdBuffer, sceneDesc, inputDescSet);

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		StaticPipes::Get<ReflprobePipe>().Draw(cmdBuffer, sceneDesc, inputDescSet);

		StaticPipes::Get<IrragridPipe>().Draw(cmdBuffer, sceneDesc, inputDescSet);
	});

	m_secondaryPassInst[sceneDesc.frameIndex].RecordPass(
		cmdBuffer, vk::SubpassContents::eInline, [&]() { StaticPipes::Get<AmbientPipe>().Draw(cmdBuffer, sceneDesc); });
}

void Renderer_::DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass)
{
	PROFILE_SCOPE(Renderer);

	UpdateGlobalDescSet(sceneDesc);

	CMDSCOPE_BEGIN(cmdBuffer, "Calculate shadow maps");

	// calculates: shadowmaps, gi maps
	// requires: -
	CalculateShadowmaps::RecordCmd(cmdBuffer, sceneDesc);

	CMDSCOPE_END(cmdBuffer);

	CMDSCOPE_BEGIN(cmdBuffer, "Bake probes");

	// TODO: Probe baking should be part of the scene maybe - on the other hand real time probes should be here
	BakeProbes::RecordCmd(sceneDesc); // NOTE: Blocking

	CMDSCOPE_END(cmdBuffer);

	CMDSCOPE_BEGIN(cmdBuffer, "Draw geometry and lights");

	// calculates: gbuffer, direct lights, gi, ambient
	// requires: shadowmaps, gi maps
	DrawGeometryAndLights(cmdBuffer, sceneDesc);

	CMDSCOPE_END(cmdBuffer);

	CMDSCOPE_BEGIN(cmdBuffer, "Raytrace area lights");

	// calculates: total of arealights
	// requires: -
	m_raytraceArealights.RecordCmd(cmdBuffer, sceneDesc);

	CMDSCOPE_END(cmdBuffer);

	CMDSCOPE_BEGIN(cmdBuffer, "Raytrace mirror reflections");

	// calculates: specular reflections (mirror)
	// requires: gbuffer, shadowmaps, gi maps
	m_raytraceMirrorReflections.RecordCmd(cmdBuffer, sceneDesc);

	CMDSCOPE_END(cmdBuffer);

	CMDSCOPE_BEGIN(cmdBuffer, "Post process commands");

	// TODO: post process
	m_ptPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&] {
		m_ptLightBlend.Draw(cmdBuffer, sceneDesc); // fixed

		// TODO: from post proc
		// m_postprocCollection.Draw(*cmdBuffer, sceneDesc);
	});

	CMDSCOPE_END(cmdBuffer);

	static ConsoleVariable<bool> cons_drawUnlit{ "r.renderer.drawUnlit", true }; // CHECK: more specific

	if (cons_drawUnlit) [[likely]] {
		CMDSCOPE_BEGIN(cmdBuffer, "Unlit pass");
		m_unlitPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&] {
			UnlitPipe::RecordCmd(cmdBuffer, sceneDesc);
			DrawSelectedEntityDebugVolume::RecordCmd(cmdBuffer, sceneDesc);
			StaticPipes::Get<BillboardPipe>().Draw(cmdBuffer, sceneDesc);
		});
		CMDSCOPE_END(cmdBuffer);
	}


	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
