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

	m_raytraceLightTest.Resize(fbSize);

	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_unlitPassInst[i] = Layouts->unlitPassLayout.CreatePassInstance(fbSize.width, fbSize.height,
			{ &m_mainPassInst[i].framebuffer[0],
				&m_raytraceLightTest.svgfRenderPassInstance.framebuffer[0] }); // TODO: indices and stuff
	}

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
		views.emplace_back(m_raytraceLightTest.svgfRenderPassInstance.framebuffer[0].view()); // WIP: -
		views.emplace_back(brdfLutImg.Lock().image.view());                                   // sceneColorSampler

		rvk::writeDescriptorImages(m_globalDesc[i], 0u, std::move(views));
		// TODO: should not rewrite, instead pass pairs with their sampler, if sampler is empty then default
		rvk::writeDescriptorImages(m_globalDesc[i], 11u, { brdfLutImg.Lock().image.view() }, brdfLutSampler);
	}

	ClearDebugAttachments();
	RegisterDebugAttachment(m_mainPassInst.at(0).framebuffer);
	RegisterDebugAttachment(m_unlitPassInst.at(0).framebuffer);
	RegisterDebugAttachment(m_raytraceLightTest.svgfRenderPassInstance.framebuffer);
	RegisterDebugAttachment(m_raytraceLightTest.svgfPass.swappingImages.at(0));
	RegisterDebugAttachment(m_raytraceLightTest.svgfPass.swappingImages.at(1));
}

InFlightResources<vk::ImageView> RtxRenderer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_raytraceLightTest.svgfRenderPassInstance.framebuffer[0].view(); // TODO: [0]
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

	m_raytraceLightTest.RecordCmd(cmdBuffer, sceneDesc);

	CMDSCOPE_END(cmdBuffer);

	static ConsoleVariable<bool> cons_drawUnlit{ "r.rtxRenderer.drawUnlit", true }; // CHECK: more specific

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
