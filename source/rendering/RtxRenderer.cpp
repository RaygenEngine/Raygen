#include "RtxRenderer.h"

#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/output/OutputPassBase.h"
#include "rendering/pipes/BillboardPipe.h"
#include "rendering/pipes/geometry/GBufferPipe.h"
#include "rendering/pipes/geometry/UnlitPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/techniques/DrawSelectedEntityDebugVolume.h"
#include "engine/Events.h"


namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

RtxRenderer_::RtxRenderer_()
{
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_globalDesc[i] = DescriptorLayouts->global.AllocDescriptorSet();
	}

	Event::OnViewerUpdated.Bind(this, [&]() { m_testTech.updateViewer.Set(); });
}

void RtxRenderer_::ResizeBuffers(uint32 width, uint32 height)
{
	vk::Extent2D fbSize = SuggestFramebufferSize(vk::Extent2D{ width, height });

	if (fbSize == m_extent) {
		return;
	}
	m_extent = fbSize;

	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		// Generate Passes
		m_mainPassInst[i] = PassLayouts->main.CreatePassInstance(fbSize.width, fbSize.height);
	}

	m_testTech.Resize(fbSize);

	for (uint32 i = 0; i < c_framesInFlight; ++i) {

		std::vector<vk::ImageView> views;

		for (auto& att : m_mainPassInst[i].framebuffer.ownedAttachments) {
			views.emplace_back(att.view());
		}

		rvk::writeDescriptorImages(m_globalDesc[i], 0u, std::move(views));

		auto [brdfLutImg, brdfLutSampler] = GpuAssetManager->GetBrdfLutImageSampler();
		rvk::writeDescriptorImages(m_globalDesc[i], DescriptorLayouts_::GlobalIndex::BrdfLut,
			{ brdfLutImg.Lock().image.view() }, brdfLutSampler);
	}

	ClearDebugAttachments();
	RegisterDebugAttachment(m_mainPassInst.at(0));
	RegisterDebugAttachment(m_testTech.pathtracedResult);
	RegisterDebugAttachment(m_testTech.progressive);
	RegisterDebugAttachment(m_testTech.momentsHistory);
	RegisterDebugAttachment(m_testTech.svgfRenderPassInstance.at(0));
}

InFlightResources<vk::ImageView> RtxRenderer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_testTech.svgfRenderPassInstance[i].framebuffer["SvgfFinalModulated"].view();
	}
	return views;
}

void RtxRenderer_::UpdateGlobalDescSet(SceneRenderDesc& sceneDesc)
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

void RtxRenderer_::DrawGeometryAndLights(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc)
{
	m_mainPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		GbufferPipe::RecordCmd(cmdBuffer, sceneDesc);

		auto inputDescSet = m_mainPassInst[sceneDesc.frameIndex].internalDescSet;

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);
	});
}

void RtxRenderer_::RecordCmd(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass)
{
	PROFILE_SCOPE(Renderer);
	COMMAND_SCOPE_AUTO(cmdBuffer);

	UpdateGlobalDescSet(sceneDesc);

	// calculates: gbuffer, direct lights, gi, ambient
	// requires: shadowmaps, gi maps
	DrawGeometryAndLights(cmdBuffer, sceneDesc);

	static ConsoleVariable<int32> cons_bounces{ "r.rtxRenderer.bounces", 1,
		"Set the number of bounces of the RTX Renderer." };
	static ConsoleVariable<int32> cons_samples{ "r.rtxRenderer.samples", 1,
		"Set the number of samples of the RTX Renderer." };
	static ConsoleVariable<float> cons_minColorAlpha{ "r.rtxRenderer.svgf.minColorAlpha", 0.05f,
		"Set SVGF color alpha for reprojection mix." };
	static ConsoleVariable<float> cons_minMomentsAlpha{ "r.rtxRenderer.svgf.minMomentsAlpha", 0.05f,
		"Set SVGF moments alpha for reprojection mix." };
	static ConsoleVariable<int32> cons_iters{ "r.rtxRenderer.svgf.iterations", 4,
		"Controls how many times to apply svgf atrous filter." };
	static ConsoleVariable<float> cons_phiColor{ "r.rtxRenderer.svgf.phiColor", 1.f, "Set atrous filter phiColor." };
	static ConsoleVariable<float> cons_phiNormal{ "r.rtxRenderer.svgf.phiNormal", 0.2f,
		"Set atrous filter phiNormal." };


	m_testTech.RecordCmd(cmdBuffer, sceneDesc, *cons_samples, *cons_bounces, *cons_minColorAlpha, *cons_minMomentsAlpha,
		*cons_iters, *cons_phiColor, *cons_phiNormal);

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
