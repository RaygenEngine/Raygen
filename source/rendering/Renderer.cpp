#include "Renderer.h"

#include "assets/StdAssets.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/output/OutputPassBase.h"
#include "rendering/pipes/AmbientPipe.h"
#include "rendering/pipes/BillboardPipe.h"
#include "rendering/pipes/DirlightPipe.h"
#include "rendering/pipes/IrragridPipe.h"
#include "rendering/pipes/PointlightPipe.h"
#include "rendering/pipes/ReflprobePipe.h"
#include "rendering/pipes/SpotlightPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/geometry/DepthmapPipe.h"
#include "rendering/pipes/geometry/GBufferPipe.h"
#include "rendering/pipes/geometry/UnlitPipe.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneIrragrid.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/techniques/CalculateIrragrids.h"
#include "rendering/techniques/CalculateReflprobes.h"
#include "rendering/techniques/CalculateShadowmaps.h"
#include "rendering/techniques/DrawSelectedEntityDebugVolume.h"
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
		m_mainPassInst[i] = Layouts->mainPassLayout.CreatePassInstance(width, height);
		m_secondaryPassInst[i] = Layouts->secondaryPassLayout.CreatePassInstance(width, height);
	}
	m_raytraceMirrorReflections.Resize(fbSize);

	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_ptPass[i] = Layouts->ptPassLayout.CreatePassInstance(
			fbSize.width, fbSize.height, { &m_mainPassInst[i].framebuffer[0] }); // TODO: indices and stuff
	}

	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_attachmentsDesc[i] = Layouts->renderAttachmentsLayout.AllocDescriptorSet();

		std::vector<vk::ImageView> views;

		for (auto& att : m_mainPassInst[i].framebuffer.ownedAttachments) {
			views.emplace_back(att.view());
		}

		// TODO: std gpu asset
		auto brdfLutImg = GpuAssetManager->GetGpuHandle(StdAssets::BrdfLut());
		views.emplace_back(m_secondaryPassInst[i].framebuffer[0].view());
		views.emplace_back(brdfLutImg.Lock().image.view()); // std_BrdfLut <- rewritten below with the correct sampler
		views.emplace_back(brdfLutImg.Lock().image.view()); // reserved
		views.emplace_back(m_raytraceMirrorReflections.result[i].view()); // mirror buffer
		views.emplace_back(m_ptPass[i].framebuffer[0].view());            // sceneColorSampler

		rvk::writeDescriptorImages(m_attachmentsDesc[i], 0u, std::move(views));

		// TODO: find an owner - std gpu asset
		vk::SamplerCreateInfo samplerInfo{};
		samplerInfo
			.setMagFilter(vk::Filter::eLinear) //
			.setMinFilter(vk::Filter::eLinear)
			.setAddressModeU(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeV(vk::SamplerAddressMode::eClampToEdge)
			.setAddressModeW(vk::SamplerAddressMode::eClampToEdge)
			.setAnisotropyEnable(VK_TRUE)
			.setMaxAnisotropy(1u)
			.setBorderColor(vk::BorderColor::eIntOpaqueBlack)
			.setUnnormalizedCoordinates(VK_FALSE)
			.setCompareEnable(VK_FALSE)
			.setCompareOp(vk::CompareOp::eAlways)
			.setMipmapMode(vk::SamplerMipmapMode::eLinear)
			.setMipLodBias(0.f)
			.setMinLod(0.f)
			.setMaxLod(32.f);
		auto brdfSampler = GpuResources::AcquireSampler(samplerInfo);

		rvk::writeDescriptorImages(m_attachmentsDesc[i], 10u, { brdfLutImg.Lock().image.view() }, brdfSampler);
	}
}

InFlightResources<vk::ImageView> Renderer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_ptPass[i].framebuffer[0].view(); // WIP: [0]
	}
	return views;
}

void Renderer_::DrawGeometryAndLights(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc)
{
	m_mainPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		GbufferPipe::RecordCmd(cmdBuffer, sceneDesc);

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<SpotlightPipe>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<SpotlightPipe>().Draw(cmdBuffer, sceneDesc);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<PointlightPipe>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<PointlightPipe>().Draw(cmdBuffer, sceneDesc);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<DirlightPipe>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<DirlightPipe>().Draw(cmdBuffer, sceneDesc);

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<ReflprobePipe>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<ReflprobePipe>().Draw(cmdBuffer, sceneDesc);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<IrragridPipe>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<IrragridPipe>().Draw(cmdBuffer, sceneDesc);
	});

	m_secondaryPassInst[sceneDesc.frameIndex].RecordPass(
		cmdBuffer, vk::SubpassContents::eInline, [&]() { StaticPipes::Get<AmbientPipe>().Draw(cmdBuffer, sceneDesc); });
}

void Renderer_::DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc, OutputPassBase& outputPass)
{
	PROFILE_SCOPE(Renderer);

	sceneDesc.attachmentsDescSet = m_attachmentsDesc[sceneDesc.frameIndex];

	// calculates: shadowmaps, gi maps
	// requires: -
	CalculateShadowmaps::RecordCmd(cmdBuffer, sceneDesc);
	CalculateReflprobes::RecordCmd(cmdBuffer, sceneDesc);
	CalculateIrragrids::RecordCmd(cmdBuffer, sceneDesc);

	// calculates: gbuffer, direct lights, gi, ambient
	// requires: shadowmaps, gi maps
	DrawGeometryAndLights(cmdBuffer, sceneDesc);

	// calculates: specular reflections (mirror)
	// requires: gbuffer, shadowmaps, gi maps
	m_raytraceMirrorReflections.RecordCmd(cmdBuffer, sceneDesc);

	// TODO: post process
	m_ptPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&] {
		m_ptLightBlend.Draw(cmdBuffer, sceneDesc); // TODO: from post proc

		// m_postprocCollection.Draw(*cmdBuffer, sceneDesc);
		UnlitPipe::RecordCmd(cmdBuffer, sceneDesc);
		DrawSelectedEntityDebugVolume::RecordCmd(cmdBuffer, sceneDesc);
		StaticPipes::Get<BillboardPipe>().Draw(cmdBuffer, sceneDesc);
	});

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
