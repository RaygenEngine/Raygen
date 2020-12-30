#include "Pathtracer.h"

#include "rendering/Layouts.h"
#include "rendering/output/OutputPassBase.h"

namespace {
vk::Extent2D SuggestFramebufferSize(vk::Extent2D viewportSize)
{
	return viewportSize;
}
} // namespace

namespace vl {

Pathtracer_::Pathtracer_()
{
	for (uint32 i = 0; i < c_framesInFlight; ++i) {
		m_globalDesc[i] = Layouts->globalDescLayout.AllocDescriptorSet();
	}
}

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
		views[i] = m_progressivePathtrace.result[i].view(); // WIP: [0]
	}
	return views;
}

void Pathtracer_::DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc, OutputPassBase& outputPass)
{
	m_progressivePathtrace.RecordCmd(cmdBuffer, sceneDesc, frame++);

	outputPass.RecordOutPass(cmdBuffer, sceneDesc.frameIndex);
}
} // namespace vl
