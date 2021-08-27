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
		m_globalDesc[i] = Layouts->globalDescLayout.AllocDescriptorSet();
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
		m_mainPassInst[i] = Layouts->mainPassLayout.CreatePassInstance(fbSize.width, fbSize.height);
	}

	m_testTech.Resize(fbSize);

	for (uint32 i = 0; i < c_framesInFlight; ++i) {

		std::vector<vk::ImageView> views;

		for (auto& att : m_mainPassInst[i].framebuffer.ownedAttachments) {
			views.emplace_back(att.view());
		}

		auto [brdfLutImg, brdfLutSampler] = GpuAssetManager->GetBrdfLutImageSampler();
		views.emplace_back(brdfLutImg.Lock().image.view());
		views.emplace_back(brdfLutImg.Lock().image.view()); // std_BrdfLut <- rewritten below with the correct sampler
		views.emplace_back(brdfLutImg.Lock().image.view()); // reserved0
		views.emplace_back(brdfLutImg.Lock().image.view()); // reserved1
		views.emplace_back(brdfLutImg.Lock().image.view()); // WIP: -
		views.emplace_back(brdfLutImg.Lock().image.view()); // sceneColorSampler

		rvk::writeDescriptorImages(m_globalDesc[i], 0u, std::move(views));
		// TODO: should not rewrite, instead pass pairs with their sampler, if sampler is empty then default
		rvk::writeDescriptorImages(m_globalDesc[i], 11u, { brdfLutImg.Lock().image.view() }, brdfLutSampler);
	}

	ClearDebugAttachments();
	RegisterDebugAttachment(m_mainPassInst.at(0).framebuffer);
	RegisterDebugAttachment(m_testTech.pathtraced);
	RegisterDebugAttachment(m_testTech.progressiveVariance);
	RegisterDebugAttachment(m_testTech.momentsHistory);
}

InFlightResources<vk::ImageView> RtxRenderer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_testTech.progressiveVariance.view(); // TODO: [0]
	}
	return views;
}

void RtxRenderer_::UpdateGlobalDescSet(SceneRenderDesc& sceneDesc)
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

void RtxRenderer_::DrawGeometryAndLights(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc)
{
	m_mainPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		GbufferPipe::RecordCmd(cmdBuffer, sceneDesc);

		auto inputDescSet = m_mainPassInst[sceneDesc.frameIndex].internalDescSet;

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);
	});
}

void RtxRenderer_::DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass)
{
	PROFILE_SCOPE(Renderer);

	UpdateGlobalDescSet(sceneDesc);

	CMDSCOPE_BEGIN(cmdBuffer, "Draw geometry and lights");

	// calculates: gbuffer, direct lights, gi, ambient
	// requires: shadowmaps, gi maps
	DrawGeometryAndLights(cmdBuffer, sceneDesc);

	CMDSCOPE_END(cmdBuffer);

	CMDSCOPE_BEGIN(cmdBuffer, "Pathtracer commands");

	static ConsoleVariable<int32> cons_bounces{ "r.rtxRenderer.bounces", 1,
		"Set the number of bounces of the RTX Renderer." };
	static ConsoleVariable<int32> cons_samples{ "r.rtxRenderer.samples", 1,
		"Set the number of samples of the RTX Renderer." };

	m_testTech.RecordCmd(cmdBuffer, sceneDesc, *cons_samples, *cons_bounces);

	CMDSCOPE_END(cmdBuffer);

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
