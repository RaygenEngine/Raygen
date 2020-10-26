#include "Renderer.h"

#include "assets/StdAssets.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/output/OutputPassBase.h"
#include "rendering/passes/DepthmapPass.h"
#include "rendering/passes/GBufferPass.h"
#include "rendering/passes/lightblend/DirlightBlend.h"
#include "rendering/passes/lightblend/PointlightBlend.h"
#include "rendering/passes/lightblend/ReflprobeBlend.h"
#include "rendering/passes/lightblend/SpotlightBlend.h"
#include "rendering/passes/UnlitPass.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneReflProbe.h"
#include "rendering/scene/SceneSpotlight.h"
#include "rendering/StaticPipes.h"
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

	m_mirrorPass.MakeRtPipeline();
	m_aoPass.MakeRtPipeline();
}

void Renderer_::RecordGeometryPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	m_gbufferInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		//
		GbufferPass::RecordCmd(cmdBuffer, sceneDesc);
	});

	auto shadowmapRenderpass = [&](auto light) {
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

		cmdBuffer.beginRenderPass(renderPassInfo, vk::SubpassContents::eSecondaryCommandBuffers);
		{
			auto buffers = m_secondaryBuffersPool.Get(sceneDesc.frameIndex);

			DepthmapPass::RecordCmd(&buffers, viewport, scissor, light->ubo.viewProj, sceneDesc);

			cmdBuffer.executeCommands({ buffers });
		}
		cmdBuffer.endRenderPass();
	};

	for (auto sl : sceneDesc->Get<SceneSpotlight>()) {
		shadowmapRenderpass(sl);
	}

	for (auto dl : sceneDesc->Get<SceneDirlight>()) {
		shadowmapRenderpass(dl);
	}
}

void Renderer_::RecordRelfprobeEnvmapPasses(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	for (auto rp : sceneDesc->Get<SceneReflprobe>()) {
		if (rp->shouldBuild.Access())
			[[unlikely]]
			{

				rp->surroundingEnv.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
					vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

				m_ptCube.RecordPass(cmdBuffer, sceneDesc, *rp);

				rp->surroundingEnv.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
					vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
					vk::PipelineStageFlagBits::eFragmentShader);

				StaticPipes::Get<IrradianceMapCalculation>().RecordPass(cmdBuffer, *rp);
				StaticPipes::Get<PrefilteredMapCalculation>().RecordPass(cmdBuffer, *rp);
			}
	}
}

void Renderer_::RecordRasterDirectPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	m_rasterDirectLightPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&]() {
		StaticPipes::Get<SpotlightBlend>().Draw(cmdBuffer, sceneDesc);
		StaticPipes::Get<PointlightBlend>().Draw(cmdBuffer, sceneDesc);
		StaticPipes::Get<DirlightBlend>().Draw(cmdBuffer, sceneDesc);
	});

	m_rasterIblPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline,
		[&]() { StaticPipes::Get<ReflprobeBlend>().Draw(cmdBuffer, sceneDesc); });
}

void Renderer_::RecordPostProcessPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	m_ptPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&] {
		// Post proc pass
		m_lightblendPass.Draw(cmdBuffer, sceneDesc); // TODO: from post proc

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

		m_rasterDirectLightPass[i]
			= Layouts->rasterDirectLightPassLayout.CreatePassInstance(fbSize.width, fbSize.height);

		m_rasterIblPass[i] = Layouts->rasterIblPassLayout.CreatePassInstance(fbSize.width, fbSize.height);

		m_ptPass[i] = Layouts->ptPassLayout.CreatePassInstance(
			fbSize.width, fbSize.height, { &m_gbufferInst[i].framebuffer[GDepth] });
	}

	m_mirrorPass.Resize(fbSize);
	m_aoPass.Resize(fbSize);


	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_attachmentsDesc[i] = Layouts->renderAttachmentsLayout.AllocDescriptorSet();

		std::vector<vk::ImageView> views;

		for (auto& att : m_gbufferInst[i].framebuffer.ownedAttachments) {
			views.emplace_back(att.view());
		}

		// WIP: standard gpu asset?
		auto brdfLutImg = GpuAssetManager->GetGpuHandle(StdAssets::BrdfLut());
		views.emplace_back(brdfLutImg.Lock().image.view()); // std_BrdfLut <- rewritten below with the correct sampler
		views.emplace_back(m_rasterDirectLightPass[i].framebuffer[0].view()); // raster_DirectLightSampler
		views.emplace_back(m_rasterIblPass[i].framebuffer[0].view());         // raster_IBLminusMirrorReflectionsSampler
		views.emplace_back(m_mirrorPass.m_indirectResult[i].view());          // ray_MirrorReflectionsSampler
		views.emplace_back(m_aoPass.m_indirectResult[i].view());              // ray_AOSampler
		views.emplace_back(m_ptPass[i].framebuffer[0].view());                // sceneColorSampler

		rvk::writeDescriptorImages(m_attachmentsDesc[i], 0u, std::move(views));

		// WIP: find an owner
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

		rvk::writeDescriptorImages(m_attachmentsDesc[i], 7u, { brdfLutImg.Lock().image.view() }, brdfSampler);
	}

	// RT images
	for (size_t i = 0; i < c_framesInFlight; i++) {
		m_mirrorPass.m_rtDescSet[i] = Layouts->singleStorageImage.AllocDescriptorSet();

		rvk::writeDescriptorImages(m_mirrorPass.m_rtDescSet[i], 0u, { m_mirrorPass.m_indirectResult[i].view() },
			nullptr, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

		m_aoPass.m_rtDescSet[i] = Layouts->singleStorageImage.AllocDescriptorSet();

		rvk::writeDescriptorImages(m_aoPass.m_rtDescSet[i], 0u, { m_aoPass.m_indirectResult[i].view() },
			vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
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
	RecordRelfprobeEnvmapPasses(cmdBuffer, sceneDesc);

	RecordGeometryPasses(cmdBuffer, sceneDesc);
	RecordRasterDirectPass(cmdBuffer, sceneDesc);


	m_mirrorPass.RecordPass(cmdBuffer, sceneDesc);
	m_aoPass.RecordPass(cmdBuffer, sceneDesc);


	RecordPostProcessPass(cmdBuffer, sceneDesc);
	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
