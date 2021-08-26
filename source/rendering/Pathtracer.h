#pragma once

#include "rendering/ppt/techniques/PtLightBlend.h"
#include "rendering/RendererBase.h"
#include "rendering/techniques/ProgressivePathtrace.h"
#include "rendering/wrappers/CmdBuffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {
inline class Pathtracer_ : public RendererBase {

public:
	Pathtracer_();

	void ResizeBuffers(uint32 width, uint32 height) override;

	void DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass) override;

	InFlightResources<vk::ImageView> GetOutputViews() const override;

	// CHECK: do we want unlit stuff here? - and/or editor stuff like icons etc? - if so we will need depth calculation
	// which is costly

private:
	vk::Extent2D m_extent{};

	ProgressivePathtrace m_progressivePathtrace;

	int32 m_frame{ 0 };

} * Pathtracer{};
} // namespace vl
