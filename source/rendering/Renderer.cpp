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


namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

Renderer_::Renderer_()
{
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
		imageSamplers.emplace_back(brdfLutImg.Lock().image.view(), brdfLutSampler);
		imageSamplers.emplace_back(m_raytraceArealights.svgFiltering.GetFilteredImageView(i)); // arealightShadowing
		imageSamplers.emplace_back(m_raytraceArealights.pathtracedResult.view());              // reserved1
		imageSamplers.emplace_back(m_raytraceMirrorReflections.result[i].view());              // mirror
		imageSamplers.emplace_back(m_ptPass[i].framebuffer["PostProcColor"].view());           // sceneColorSampler

		rvk::writeDescriptorImages(m_globalDesc[i], 0u, std::move(imageSamplers));
	}

	ClearDebugAttachments();
	RegisterDebugAttachment(m_mainPassInst.at(0));
	RegisterDebugAttachment(m_secondaryPassInst.at(0));
	RegisterDebugAttachment(m_ptPass.at(0));
	RegisterDebugAttachment(m_raytraceMirrorReflections.result.at(0));
	RegisterDebugAttachment(m_raytraceArealights.pathtracedResult);
	RegisterDebugAttachment(m_raytraceArealights.svgFiltering.progressive);
	RegisterDebugAttachment(m_raytraceArealights.svgFiltering.momentsHistory);
	RegisterDebugAttachment(m_raytraceArealights.svgFiltering.svgfRenderPassInstance.at(0));
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
		//
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

	static ConsoleVariable<float> cons_bias{ "r.renderer.ao.bias", 0.001f, "Set the ambient occlusion bias." };
	static ConsoleVariable<float> cons_strength{ "r.renderer.ao.strength", 1.0f,
		"Set the ambient occlusion strength." };
	static ConsoleVariable<float> cons_radius{ "r.renderer.ao.radius", .2f, "Set the ambient occlusion radius." };
	static ConsoleVariable<int32> cons_samples{ "r.renderer.ao.samples", 4, "Set the ambient occlusion samples." };

	m_secondaryPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		StaticPipes::Get<AmbientPipe>().RecordCmd(
			cmdBuffer, sceneDesc, *cons_samples, *cons_bias, *cons_strength, cons_radius);
	});
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

	static ConsoleVariable<int32> cons_samples{ "r.renderer.arealights.samples", 1,
		"Set the sample count of arealights." };
	static ConsoleVariable<float> cons_minColorAlpha{ "r.renderer.arealights.svgf.minColorAlpha", 0.05f,
		"Set SVGF color alpha for reprojection mix." };
	static ConsoleVariable<float> cons_minMomentsAlpha{ "r.renderer.arealight.svgf.minMomentsAlpha", 0.05f,
		"Set SVGF moments alpha for reprojection mix." };
	static ConsoleVariable<int32> cons_iters{ "r.renderer.arealights.svgf.iterations", 4,
		"Controls how many times to apply svgf atrous filter." };
	static ConsoleVariable<float> cons_phiColor{ "r.renderer.arealights.svgf.phiColor", 1.f,
		"Set atrous filter phiColor." };
	static ConsoleVariable<float> cons_phiNormal{ "r.renderer.arealights.svgf.phiNormal", 0.2f,
		"Set atrous filter phiNormal." };
	// calculates: total of arealights
	// requires: -
	m_raytraceArealights.RecordCmd(cmdBuffer, sceneDesc, *cons_samples, *cons_minColorAlpha, *cons_minMomentsAlpha,
		*cons_iters, *cons_phiColor, *cons_phiNormal);

	static ConsoleVariable<int32> cons_bounces{ "r.renderer.mirror.bounces", 1,
		"Set the number of bounces of mirror reflections." };

	// calculates: specular reflections (mirror)
	// requires: gbuffer, shadowmaps, gi maps
	m_raytraceMirrorReflections.RecordCmd(cmdBuffer, sceneDesc, *cons_bounces);

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
