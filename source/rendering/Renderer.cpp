#include "Renderer.h"

#include "assets/StdAssets.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/StaticPipes.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/output/OutputPassBase.h"
#include "rendering/passes/direct/DirlightBlend.h"
#include "rendering/passes/direct/PointlightBlend.h"
#include "rendering/passes/direct/SpotlightBlend.h"
#include "rendering/passes/geometry/DepthmapPass.h"
#include "rendering/passes/geometry/GBufferPass.h"
#include "rendering/passes/gi/AmbientBlend.h"
#include "rendering/passes/gi/IrragridBlend.h"
#include "rendering/passes/gi/ReflprobeBlend.h"
#include "rendering/passes/unlit/UnlitBillboardPass.h"
#include "rendering/passes/unlit/UnlitGeometryPass.h"
#include "rendering/passes/unlit/UnlitSelectionStencilPass.h"
#include "rendering/passes/unlit/UnlitVolumePass.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneIrragrid.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/scene/SceneSpotlight.h"
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

	m_lightblendPass.MakeLayout();
	m_lightblendPass.MakePipeline();

	m_indirectSpecPass.MakeRtPipeline();
	m_mirorPass.MakeRtPipeline();
}

void Renderer_::RecordMapPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	for (auto sl : sceneDesc->Get<SceneSpotlight>()) {
		if (sl->ubo.hasShadow) {
			sl->shadowmapPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
				//
				DepthmapPass::RecordCmd(cmdBuffer, sl->ubo.viewProj, sceneDesc);
			});
		}
	}

	for (auto dl : sceneDesc->Get<SceneDirlight>()) {
		if (dl->ubo.hasShadow) {
			dl->shadowmapPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
				//
				DepthmapPass::RecordCmd(cmdBuffer, dl->ubo.viewProj, sceneDesc);
			});
		}
	}

	for (auto rp : sceneDesc->Get<SceneReflprobe>()) {
		if (rp->shouldBuild.Access()) [[unlikely]] {
			rp->environment.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

			m_ptCubemap.RecordPass(cmdBuffer, sceneDesc, *rp);

			rp->environment.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
				vk::PipelineStageFlagBits::eComputeShader);

			rp->irradiance.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

			// CHECK: use compute buffer
			m_compCubemapConvolution.RecordPass(cmdBuffer, sceneDesc, *rp);

			rp->irradiance.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eFragmentShader);

			rp->prefiltered.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

			m_compPrefilteredConvolution.RecordPass(cmdBuffer, sceneDesc, *rp);

			rp->prefiltered.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eFragmentShader);
		}
	}

	for (auto ig : sceneDesc->Get<SceneIrragrid>()) {
		if (ig->shouldBuild.Access()) [[unlikely]] {

			ig->environmentCubemaps.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined,
				vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eTopOfPipe,
				vk::PipelineStageFlagBits::eRayTracingShaderKHR);

			m_ptCubemapArray.RecordPass(cmdBuffer, sceneDesc, *ig);

			ig->environmentCubemaps.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
				vk::PipelineStageFlagBits::eComputeShader);

			ig->irradianceCubemaps.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eComputeShader);

			// CHECK: use compute buffer
			m_compCubemapArrayConvolution.RecordPass(cmdBuffer, sceneDesc, *ig);

			ig->irradianceCubemaps.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eComputeShader,
				vk::PipelineStageFlagBits::eFragmentShader);
		}
	}
}

void Renderer_::RecordMainPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	m_mainPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		GbufferPass::RecordCmd(cmdBuffer, sceneDesc);

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<SpotlightBlend>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<SpotlightBlend>().Draw(cmdBuffer, sceneDesc);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<PointlightBlend>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<PointlightBlend>().Draw(cmdBuffer, sceneDesc);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<DirlightBlend>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<DirlightBlend>().Draw(cmdBuffer, sceneDesc);

		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<ReflprobeBlend>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<ReflprobeBlend>().Draw(cmdBuffer, sceneDesc);

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<IrragridBlend>().layout(), 0u,
			1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<IrragridBlend>().Draw(cmdBuffer, sceneDesc);
	});
}

void Renderer_::RecordSecondaryPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	m_secondaryPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline,
		[&]() { StaticPipes::Get<AmbientBlend>().Draw(cmdBuffer, sceneDesc); });

	// m_mirorPass.RecordPass(cmdBuffer, sceneDesc);

	// WIP: decide, also can it be merged with mirror pass?
	// m_indirectSpecPass.RecordPass(cmdBuffer, sceneDesc);
}

void Renderer_::RecordPostProcessPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	m_ptPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&] {
		m_lightblendPass.Draw(cmdBuffer, sceneDesc); // TODO: from post proc

		// m_postprocCollection.Draw(*cmdBuffer, sceneDesc);
		UnlitGeometryPass::RecordCmd(cmdBuffer, sceneDesc);
		StaticPipes::Get<UnlitVolumePass>().Draw(cmdBuffer, sceneDesc);
		StaticPipes::Get<UnlitSelectionStencilPass>().Draw(cmdBuffer, sceneDesc);
		StaticPipes::Get<UnlitBillboardPass>().Draw(cmdBuffer, sceneDesc);

		// vk::ClearDepthStencilValue ccv{};
		// ccv.depth = 1.f;
		// ccv.stencil = 0;

		// vk::ImageSubresourceRange is{};
		// is.setAspectMask(vk::ImageAspectFlagBits::eDepth | vk::ImageAspectFlagBits::eStencil) //
		//	.setBaseMipLevel(0u)
		//	.setLevelCount(1u)
		//	.setBaseArrayLayer(0u)
		//	.setLayerCount(1u);

		// cmdBuffer.clearDepthStencilImage(m_mainPassInst[sceneDesc.frameIndex].framebuffer[0].handle(),
		//	vk::ImageLayout::eShaderReadOnlyOptimal, ccv, is);

		StaticPipes::Get<UnlitSelectionStencilPass>().Draw(cmdBuffer, sceneDesc);
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
		m_mainPassInst[i] = Layouts->mainPassLayout.CreatePassInstance(fbSize.width, fbSize.height);
		m_secondaryPassInst[i] = Layouts->secondaryPassLayout.CreatePassInstance(fbSize.width, fbSize.height);
		m_ptPass[i] = Layouts->ptPassLayout.CreatePassInstance(
			fbSize.width, fbSize.height, { &m_mainPassInst[i].framebuffer[0] }); // TODO: indices and stuff
	}

	m_indirectSpecPass.Resize(fbSize);
	m_mirorPass.Resize(fbSize);

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
		views.emplace_back(m_indirectSpecPass.m_svgfRenderPassInstance[i].framebuffer[0].view()); // indirect spec pass
		views.emplace_back(m_mirorPass.m_result[i].view());                                       // mirror buffer
		views.emplace_back(m_ptPass[i].framebuffer[0].view());                                    // sceneColorSampler

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

void Renderer_::DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc, OutputPassBase& outputPass)
{
	PROFILE_SCOPE(Renderer);

	sceneDesc.attachmentsDescSet = m_attachmentsDesc[sceneDesc.frameIndex];

	// shadowmaps and baked indirect light
	RecordMapPasses(cmdBuffer, sceneDesc);

	// gbuffer, direct light and baked indirect light
	RecordMainPass(cmdBuffer, sceneDesc);

	// additional gi passes
	RecordSecondaryPasses(cmdBuffer, sceneDesc);

	// post process
	RecordPostProcessPasses(cmdBuffer, sceneDesc);

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
