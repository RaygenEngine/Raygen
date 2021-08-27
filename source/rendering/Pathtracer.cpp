#include "Pathtracer.h"

#include "rendering/output/OutputPassBase.h"
#include "rendering/scene/SceneCamera.h"
#include "rendering/pipes/StaticPipes.h"
#include "rendering/pipes/AccumulationPipe.h"
#include "engine/Events.h"

namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {
Pathtracer_::Pathtracer_()
{
	Event::OnViewerUpdated.Bind(this, [&]() {
		m_progressivePathtrace.iteration = 0;
		m_progressivePathtrace.updateViewer.Set();
	});
}

void Pathtracer_::ResizeBuffers(uint32 width, uint32 height)
{
	vk::Extent2D fbSize = SuggestFramebufferSize(vk::Extent2D{ width, height });

	if (fbSize == m_extent) {
		return;
	}
	m_extent = fbSize;

	m_progressivePathtrace.Resize(fbSize);

	ClearDebugAttachments();
	RegisterDebugAttachment(m_progressivePathtrace.pathtraced);
	RegisterDebugAttachment(m_progressivePathtrace.progressive);
}

InFlightResources<vk::ImageView> Pathtracer_::GetOutputViews() const
{
	InFlightResources<vk::ImageView> views;
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_progressivePathtrace.progressive.view();
	}
	return views;
}

void Pathtracer_::DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass)
{
	CMDSCOPE_BEGIN(cmdBuffer, "Progressive Pathtrace");

	m_progressivePathtrace.RecordCmd(cmdBuffer, sceneDesc);

	CMDSCOPE_END(cmdBuffer);

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
