#pragma once

#include "engine/Listener.h"
#include "rendering/Renderer.h" // WIP: RendereBase.h
#include "rendering/ppt/techniques/PtLightBlend.h"
#include "rendering/techniques/ProgressivePathtrace.h"
#include "rendering/wrappers/CmdBuffer.h"
#include "rendering/wrappers/passlayout/RenderPassLayout.h"

namespace vl {
class OutputPassBase;
}
struct SceneRenderDesc;

namespace vl {

inline class Pathtracer_ : public RendererBase {

public:
	Pathtracer_();

	void ResizeBuffers(uint32 width, uint32 height) override;

	void DrawFrame(vk::CommandBuffer cmdBuffer, SceneRenderDesc& sceneDesc, OutputPassBase& outputPass) override;

	InFlightResources<vk::ImageView> GetOutputViews() const override;

	// CHECK: do we want unlit stuff here? - and/or editor stuff like icons etc?

	// non-static techniques
	ProgressivePathtrace m_progressivePathtrace;

private:
	vk::Extent2D m_extent{};

	InFlightResources<size_t> viewerId{ ~0llu };
	InFlightResources<vk::DescriptorSet> m_globalDesc;

} * Pathtracer{};
} // namespace vl
