#include "Pathtracer.h"

#include "rendering/output/OutputPassBase.h"
#include "rendering/scene/SceneCamera.h"

namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

void Pathtracer_::ResizeBuffers(uint32 width, uint32 height)
{
	vk::Extent2D fbSize = SuggestFramebufferSize(vk::Extent2D{ width, height });

	if (fbSize == m_extent) {
		return;
	}
	m_extent = fbSize;

	m_progressivePathtrace.Resize(fbSize);
}

InFlightResources<vk::ImageView> Pathtracer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_progressivePathtrace.progressive.view(); // WIP: [0]
	}
	return views;
}

void Pathtracer_::DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass)
{
	// PERF: only if camera is dirty
	m_progressivePathtrace.UpdateViewer(sceneDesc.viewer.ubo.viewInv, sceneDesc.viewer.ubo.projInv, 0.0);

	m_progressivePathtrace.RecordCmd(cmdBuffer, sceneDesc, m_frame++);

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
