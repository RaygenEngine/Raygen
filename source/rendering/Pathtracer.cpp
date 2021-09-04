#include "Pathtracer.h"

#include "rendering/output/OutputPassBase.h"
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
	Event::OnViewerUpdated.Bind(this, [&]() { m_progressivePathtrace.updateViewer.Set(); });
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
	for (size_t i = 0; i < c_framesInFlight; ++i) {
		views[i] = m_progressivePathtrace.progressive.view();
	}
	return views;
}

void Pathtracer_::RecordCmd(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass)
{
	COMMAND_SCOPE_AUTO(cmdBuffer);

	static ConsoleVariable<int32> cons_bounces{ "r.pathtracer.bounces", 1,
		"Set the number of bounces of the pathtracer." };
	static ConsoleVariable<int32> cons_samples{ "r.pathtracer.samples", 1,
		"Set the number of samples of the pathtracer." };
	static ConsoleVariable<PtMode> cons_mode{ "r.pathtracer.mode", PtMode::Stochastic,
		"Set the pathtracing mode. Naive: unidirectional, not entirely naive, it just lacks direct light sampling,"
		"Stochastic: similar to naive but uses direct light and light MIS,"
		"Bpdt: Bidirectional:... Work in progress" };

	m_progressivePathtrace.RecordCmd(cmdBuffer, sceneDesc, *cons_samples, *cons_bounces, *cons_mode);

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
