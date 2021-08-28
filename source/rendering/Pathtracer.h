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

	void RecordCmd(vk::CommandBuffer cmdBuffer, SceneRenderDesc&& sceneDesc, OutputPassBase& outputPass) override;

	InFlightResources<vk::ImageView> GetOutputViews() const override;

private:
	vk::Extent2D m_extent{};

	ProgressivePathtrace m_progressivePathtrace;
} * Pathtracer{};
} // namespace vl
