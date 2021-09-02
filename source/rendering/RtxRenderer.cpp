#include "RtxRenderer.h"

#include "engine/Events.h"
#include "engine/profiler/ProfileScope.h"
#include "rendering/assets/GpuAssetManager.h"
#include "rendering/assets/GpuImage.h"
#include "rendering/output/OutputPassBase.h"
#include "rendering/pipes/BillboardPipe.h"
#include "rendering/pipes/geometry/GBufferPipe.h"
#include "rendering/pipes/geometry/UnlitPipe.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/StochasticPathtracePipe.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/techniques/DrawSelectedEntityDebugVolume.h"


namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}

struct UBO_viewer {
	glm::mat4 viewInv;
	glm::mat4 projInv;
	float offset;
};

} // namespace

namespace vl {

RtxRenderer_::RtxRenderer_()
{
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_globalDesc[i] = DescriptorLayouts->global.AllocDescriptorSet();
	}

	pathtracingInputDescSet = DescriptorLayouts->_1storageImage.AllocDescriptorSet();
	DEBUG_NAME_AUTO(pathtracingInputDescSet);

	viewerDescSet = DescriptorLayouts->_1uniformBuffer.AllocDescriptorSet();
	DEBUG_NAME_AUTO(viewerDescSet);

	auto uboSize = sizeof(UBO_viewer);

	viewer = vl::RBuffer{ uboSize, vk::BufferUsageFlagBits::eUniformBuffer,
		vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent };

	rvk::writeDescriptorBuffer(viewerDescSet, 0u, viewer.handle());

	Event::OnViewerUpdated.Bind(this, [&]() { updateViewer.Set(); });
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

	pathtracedResult = RImage2D("Pathtraced (per iteration)", vk::Extent2D{ fbSize.width, fbSize.height },
		vk::Format::eR32G32B32A32Sfloat, vk::ImageLayout::eShaderReadOnlyOptimal);

	m_svgFiltering.AttachInputImage(pathtracedResult);

	rvk::writeDescriptorImages(pathtracingInputDescSet, 0u, { pathtracedResult.view() },
		vk::DescriptorType::eStorageImage, vk::ImageLayout::eGeneral);

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
	RegisterDebugAttachment(pathtracedResult);
	RegisterDebugAttachment(m_svgFiltering.progressive);
	RegisterDebugAttachment(m_svgFiltering.momentsHistory);
	RegisterDebugAttachment(m_svgFiltering.svgfRenderPassInstance.at(0));
}

InFlightResources<vk::ImageView> RtxRenderer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_svgFiltering.GetFilteredImageView(i);
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
		cmdBuffer.nextSubpass(vk::SubpassContents::eInline); // TODO:
		cmdBuffer.nextSubpass(vk::SubpassContents::eInline);
	});
}

void RtxRenderer_::RecordCmd(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass)
{
	PROFILE_SCOPE(Renderer);
	COMMAND_SCOPE_AUTO(cmdBuffer);

	if (updateViewer.Access()) {
		UBO_viewer data = {
			sceneDesc.viewer.ubo.viewInv,
			sceneDesc.viewer.ubo.projInv,
			0,
		};

		viewer.UploadData(&data, sizeof(UBO_viewer));
	}

	auto extent = pathtracedResult.extent;

	// Pathtracing
	pathtracedResult.TransitionToLayout(cmdBuffer, vk::ImageLayout::eShaderReadOnlyOptimal, vk::ImageLayout::eGeneral,
		vk::PipelineStageFlagBits::eFragmentShader, vk::PipelineStageFlagBits::eRayTracingShaderKHR);

	static int32 seed = 0;
	static ConsoleVariable<int32> cons_bounces{ "r.rtxRenderer.bounces", 1,
		"Set the number of bounces of the RTX Renderer." };
	static ConsoleVariable<int32> cons_samples{ "r.rtxRenderer.samples", 1,
		"Set the number of samples of the RTX Renderer." };

	StaticPipes::Get<StochasticPathtracePipe>().RecordCmd(
		cmdBuffer, sceneDesc, extent, pathtracingInputDescSet, viewerDescSet, seed++, *cons_samples, *cons_bounces);

	pathtracedResult.TransitionToLayout(cmdBuffer, vk::ImageLayout::eGeneral, vk::ImageLayout::eShaderReadOnlyOptimal,
		vk::PipelineStageFlagBits::eRayTracingShaderKHR, vk::PipelineStageFlagBits::eFragmentShader);

	// SVGF
	UpdateGlobalDescSet(sceneDesc);

	// calculates: gbuffer, direct lights, gi, ambient
	// requires: shadowmaps, gi maps
	DrawGeometryAndLights(cmdBuffer, sceneDesc);

	static ConsoleVariable<float> cons_minColorAlpha{ "r.rtxRenderer.svgf.minColorAlpha", 0.05f,
		"Set SVGF color alpha for reprojection mix." };
	static ConsoleVariable<float> cons_minMomentsAlpha{ "r.rtxRenderer.svgf.minMomentsAlpha", 0.05f,
		"Set SVGF moments alpha for reprojection mix." };
	static ConsoleVariable<int32> cons_iters{ "r.rtxRenderer.svgf.iterations", 4,
		"Controls how many times to apply svgf atrous filter." };
	static ConsoleVariable<float> cons_phiColor{ "r.rtxRenderer.svgf.phiColor", 1.f, "Set atrous filter phiColor." };
	static ConsoleVariable<float> cons_phiNormal{ "r.rtxRenderer.svgf.phiNormal", 0.2f,
		"Set atrous filter phiNormal." };

	m_svgFiltering.RecordCmd(cmdBuffer, sceneDesc, *cons_minColorAlpha, *cons_minMomentsAlpha, *cons_iters,
		*cons_phiColor, *cons_phiNormal, true);

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
