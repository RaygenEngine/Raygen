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
#include "rendering/passes/gi/IrradianceGridBlend.h"
#include "rendering/passes/gi/ReflprobeBlend.h"
#include "rendering/passes/unlit/UnlitBillboardPass.h"
#include "rendering/passes/unlit/UnlitGeometryPass.h"
#include "rendering/passes/unlit/UnlitVolumePass.h"
#include "rendering/resource/GpuResources.h"
#include "rendering/scene/SceneDirlight.h"
#include "rendering/scene/SceneIrradianceGrid.h"
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
			rp->surroundingEnv.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined, vk::ImageLayout::eGeneral,
				vk::PipelineStageFlagBits::eTopOfPipe, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

			PtCubeInfo ptInfo{
				rp->surroundingEnv.extent.width,
				rp->position,
				rp->innerRadius,
				rp->ptSamples,
				rp->ptBounces,
				rp->ptcube_faceArrayDescSet,
			};

			m_ptCube.RecordPass(cmdBuffer, sceneDesc, ptInfo);

			rp->surroundingEnv.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
				vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
				vk::PipelineStageFlagBits::eFragmentShader);

			CalcIrrInfo info{
				rp->irradiance.extent.width,
				vk::uniqueToRaw(rp->irr_framebuffer),
				rp->surroundingEnvSamplerDescSet,
			};

			StaticPipes::Get<IrradianceMapCalculation>().RecordPass(cmdBuffer, info);
			StaticPipes::Get<PrefilteredMapCalculation>().RecordPass(cmdBuffer, *rp);
		}
	}

	for (auto ig : sceneDesc->Get<SceneIrradianceGrid>()) {
		if (ig->shouldBuild.Access()) [[unlikely]] {
			for (int32 x = 0; x < ig->ubo.width; ++x) {
				for (int32 y = 0; y < ig->ubo.height; ++y) {
					for (int32 z = 0; z < ig->ubo.depth; ++z) {

						int32 i = 0;
						i += x;
						i += y * ig->ubo.width;
						i += z * ig->ubo.width * ig->ubo.height;


						ig->probes[i].surroundingEnv.TransitionToLayout(cmdBuffer, vk::ImageLayout::eUndefined,
							vk::ImageLayout::eGeneral, vk::PipelineStageFlagBits::eTopOfPipe,
							vk::PipelineStageFlagBits::eRayTracingShaderKHR);

						auto worldPos = glm::vec3(ig->ubo.posAndDist) + (glm::vec3(x, y, z) * ig->ubo.posAndDist.w);

						PtCubeInfo ptInfo{
							ig->probes[i].surroundingEnv.extent.width,
							glm::vec4(worldPos, 1.f), // grid block centers
							0.f,
							ig->ptSamples,
							ig->ptBounces,
							ig->probes[i].ptcube_faceArrayDescSet,
						};


						m_ptCube.RecordPass(cmdBuffer, sceneDesc, ptInfo);

						ig->probes[i].surroundingEnv.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral,
							vk::ImageLayout::eShaderReadOnlyOptimal, vk::PipelineStageFlagBits::eRayTracingShaderKHR,
							vk::PipelineStageFlagBits::eFragmentShader);

						CalcIrrInfo info{
							ig->probes[i].irradiance.extent.width,
							vk::uniqueToRaw(ig->probes[i].irr_framebuffer),
							ig->probes[i].surroundingEnvSamplerDescSet,
						};

						StaticPipes::Get<IrradianceMapCalculation>().RecordPass(cmdBuffer, info);
					}
				}
			}
		}
	} // namespace vl
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

		cmdBuffer.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, StaticPipes::Get<IrradianceGridBlend>().layout(),
			0u, 1u, &m_mainPassInst[sceneDesc.frameIndex].internalDescSet, 0u, nullptr);

		StaticPipes::Get<IrradianceGridBlend>().Draw(cmdBuffer, sceneDesc);
	});
}

void Renderer_::RecordSecondaryPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	m_secondaryPassInst[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline,
		[&]() { StaticPipes::Get<AmbientBlend>().Draw(cmdBuffer, sceneDesc); });
}

void Renderer_::RecordPostProcessPass(vk::CommandBuffer cmdBuffer, const SceneRenderDesc& sceneDesc)
{
	PROFILE_SCOPE(Renderer);

	m_ptPass[sceneDesc.frameIndex].RecordPass(cmdBuffer, vk::SubpassContents::eInline, [&] {
		m_lightblendPass.Draw(cmdBuffer, sceneDesc); // TODO: from post proc

		// m_postprocCollection.Draw(*cmdBuffer, sceneDesc);
		UnlitGeometryPass::RecordCmd(cmdBuffer, sceneDesc);
		StaticPipes::Get<UnlitVolumePass>().Draw(cmdBuffer, sceneDesc);
		StaticPipes::Get<UnlitBillboardPass>().Draw(cmdBuffer, sceneDesc);
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
		views.emplace_back(m_indirectSpecPass.m_result[i].view()); // TODO: reserved0
		views.emplace_back(m_ptPass[i].framebuffer[0].view());     // sceneColorSampler

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

	// RT images
	for (size_t i = 0; i < c_framesInFlight; i++) {
		m_indirectSpecPass.m_rtDescSet[i] = Layouts->singleStorageImage.AllocDescriptorSet();

		rvk::writeDescriptorImages(m_indirectSpecPass.m_rtDescSet[i], 0u, { m_indirectSpecPass.m_result[i].view() },
			nullptr, vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);
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

	sceneDesc.attachmentsDescSet = m_attachmentsDesc[sceneDesc.frameIndex];

	RecordMapPasses(cmdBuffer, sceneDesc);

	RecordMainPass(cmdBuffer, sceneDesc);

	RecordSecondaryPass(cmdBuffer, sceneDesc);

	m_indirectSpecPass.RecordPass(cmdBuffer, sceneDesc);

	RecordPostProcessPass(cmdBuffer, sceneDesc);
	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
